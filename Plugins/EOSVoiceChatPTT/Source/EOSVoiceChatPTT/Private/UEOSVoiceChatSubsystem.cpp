#include "UEOSVoiceChatSubsystem.h"

#ifndef WITH_SLATE_APPLICATION
#define WITH_SLATE_APPLICATION 0
#endif

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "VoiceChat.h"
#include "Containers/Ticker.h"
#include "InputCoreTypes.h"
#if WITH_SLATE_APPLICATION
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#endif
#include "Misc/PackageName.h"
#include "Misc/Crc.h"
#include "UObject/UObjectGlobals.h"

#include "IOnlineSubsystemEOS.h"
#include "IEOSSDKManager.h"
#include "EOSVoiceChatUser.h"

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
#include "eos_sdk.h"
#include "eos_connect.h"
#include "eos_lobby.h"
#include "eos_rtc.h"
#include "eos_rtc_audio.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogEOSVoiceChatSubsystem, Log, All);

#if WITH_SLATE_APPLICATION
class FPTTInputPreProcessor : public IInputProcessor
{
public:
	explicit FPTTInputPreProcessor(UEOSVoiceChatSubsystem& InSubsystem)
		: Subsystem(InSubsystem)
	{
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		if (InKeyEvent.GetKey() != EKeys::V)
		{
			return false;
		}

		if (!GEngine || !GEngine->GameViewport || !GEngine->GameViewport->Viewport)
		{
			return false;
		}

		if (!GEngine->GameViewport->Viewport->HasFocus())
		{
			return false;
		}

		Subsystem.OnPTTPressed();
		return false;
	}

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		if (InKeyEvent.GetKey() != EKeys::V)
		{
			return false;
		}

		if (!GEngine || !GEngine->GameViewport || !GEngine->GameViewport->Viewport)
		{
			return false;
		}

		if (!GEngine->GameViewport->Viewport->HasFocus())
		{
			return false;
		}

		Subsystem.OnPTTReleased();
		return false;
	}

private:
	UEOSVoiceChatSubsystem& Subsystem;
};
#endif

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
static const TCHAR* EosResultToString(EOS_EResult Result)
{
	return UTF8_TO_TCHAR(EOS_EResult_ToString(Result));
}
#endif

UEOSVoiceChatSubsystem::UEOSVoiceChatSubsystem()
	: VoiceChat(nullptr)
	, VoiceChatUser(nullptr)
	, bVoiceInitialized(false)
	, bVoiceConnected(false)
	, bVoiceLoggedIn(false)
	, bPTTEnabled(true)
	, bAlwaysTransmit(false)
	, bMuted(false)
	, bIsPTTActive(false)
	, bVoiceIndicatorActive(false)
	, bPendingAutoJoin(false)
	, bVoiceLobbyInFlight(false)
	, LastAutoJoinAttemptTime(0.0)
	, LastMicLogTime(0.0)
	, AutoJoinRetryInterval(1.0f)
	, ChannelMode(EEOSVoiceChannelMode::None)
	, EnhancedPressedBindingHandle(0)
	, EnhancedReleasedBindingHandle(0)
	, EnhancedCanceledBindingHandle(0)
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	, EOSConnectHandle(nullptr)
	, EOSLobbyHandle(nullptr)
	, EOSRtcHandle(nullptr)
	, EOSRtcAudioHandle(nullptr)
	, LocalProductUserId(nullptr)
	, bManualRtcJoined(false)
	, bConnectLoginInFlight(false)
	, RtcBeforeSendNotifyId(0)
#endif
{
	FallbackChannelName = TEXT("eos_voice_default");
	bAutoJoinOnLogin = true;
	bEnablePTTByDefault = true;
	bAlwaysTransmitByDefault = false;
	MappingPriority = 1000;
	bPreferLobbyRtc = true;
	bEnableVoiceLobby = true;
	VoiceLobbyIdPrefix = TEXT("VOICE");
	VoiceLobbyBucketId = TEXT("voice");
	VoiceLobbyMaxMembers = 16;
	AutoJoinMapName = TEXT("Redwood");
	bEnableMicActivityLogs = true;
	MicActivityLogIntervalSeconds = 0.5f;
	MicActivityAmplitudeThreshold = 500;

	ManualClientBaseUrl = TEXT("");
	ManualParticipantToken = TEXT("");
	ManualChannelName = TEXT("eos_voice_fallback");
}

void UEOSVoiceChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IsRunningDedicatedServer())
	{
		return;
	}

	LoadConfig();
	bPTTEnabled = bEnablePTTByDefault;
	bAlwaysTransmit = bAlwaysTransmitByDefault;
	InitializeVoiceChat();
	UpdateVoiceIndicatorFromState();

	if (!MapLoadedHandle.IsValid())
	{
		MapLoadedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddLambda([this](UWorld* LoadedWorld)
		{
			if (!LoadedWorld)
			{
				return;
			}

			const FString MapAssetName = FPackageName::GetShortName(LoadedWorld->GetOutermost()->GetName());
			const FString MapName = UWorld::RemovePIEPrefix(MapAssetName);
			if (AutoJoinMapName.IsEmpty() || !MapName.Equals(AutoJoinMapName, ESearchCase::IgnoreCase))
			{
				if (!AutoJoinMapName.IsEmpty())
				{
				}

				if (ChannelMode == EEOSVoiceChannelMode::LobbyRtc || ChannelMode == EEOSVoiceChannelMode::LobbyRtcSdk)
				{
					LeaveChannel();
				}

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
				LeaveVoiceLobby();
				if (bVoiceLobbyInFlight)
				{
					bVoiceLobbyInFlight = false;
					VoiceLobbyIdOverride.Reset();
					ActiveLobbyId.Reset();
				}
#endif

				// Reset voice transmission state when leaving the auto-join map.
				bPTTEnabled = bEnablePTTByDefault;
				bAlwaysTransmit = bAlwaysTransmitByDefault;
				bIsPTTActive = false;
				UpdateRTCSending(false);
				UpdateVoiceIndicatorFromState();

				return;
			}

			bPendingAutoJoin = true;
			EnsureAutoJoinTicker();
		});
	}

	if (!InputTickHandle.IsValid())
	{
		InputTickHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UEOSVoiceChatSubsystem::TickBindInput),
			0.1f);
	}
}

void UEOSVoiceChatSubsystem::Deinitialize()
{
	UnbindInput();

	UpdateRTCSending(false);

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	UnregisterRtcAudioNotify();
	LeaveVoiceLobby();
#endif

	if (MapLoadedHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadedHandle);
		MapLoadedHandle.Reset();
	}

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (ChannelMode == EEOSVoiceChannelMode::ManualRtc && EOSRtcHandle && LocalProductUserId && !ActiveRoomName.IsEmpty())
	{
		const FTCHARToUTF8 RoomNameUtf8(*ActiveRoomName);
		EOS_RTC_LeaveRoomOptions LeaveOptions = {};
		LeaveOptions.ApiVersion = EOS_RTC_LEAVEROOM_API_LATEST;
		LeaveOptions.LocalUserId = LocalProductUserId;
		LeaveOptions.RoomName = RoomNameUtf8.Get();

		EOS_RTC_LeaveRoom(EOSRtcHandle, &LeaveOptions, this, &UEOSVoiceChatSubsystem::OnRTCLeaveRoomStatic);
	}
#endif

	if (VoiceChat)
	{
		if (VoiceChatUser && VoiceChatUser->IsLoggedIn())
		{
			VoiceChatUser->Logout(FOnVoiceChatLogoutCompleteDelegate::CreateLambda(
				[](const FString& PlayerName, const FVoiceChatResult& Result)
				{
				}));
		}

		if (VoiceChatUser && ChannelJoinedLogHandle.IsValid())
		{
			VoiceChatUser->OnVoiceChatChannelJoined().Remove(ChannelJoinedLogHandle);
			ChannelJoinedLogHandle.Reset();
		}

		if (VoiceChatUser && VoiceCaptureLogHandle.IsValid())
		{
			VoiceChatUser->UnregisterOnVoiceChatAfterCaptureAudioReadDelegate(VoiceCaptureLogHandle);
			VoiceCaptureLogHandle.Reset();
		}

		if (VoiceChat->IsConnected())
		{
			VoiceChat->Disconnect(FOnVoiceChatDisconnectCompleteDelegate::CreateLambda(
				[](const FVoiceChatResult& Result)
				{
				}));
		}

		if (VoiceChat->IsInitialized())
		{
			VoiceChat->Uninitialize(FOnVoiceChatUninitializeCompleteDelegate::CreateLambda(
				[](const FVoiceChatResult& Result)
				{
				}));
		}

		if (VoiceChatUser)
		{
			VoiceChat->ReleaseUser(VoiceChatUser);
			VoiceChatUser = nullptr;
		}
	}

	if (InputTickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(InputTickHandle);
		InputTickHandle.Reset();
	}

	Super::Deinitialize();
}

void UEOSVoiceChatSubsystem::ToggleMute(bool bMute)
{
	bMuted = bMute;
	UpdateRTCSending(false);
	UpdateVoiceIndicatorFromState();

	if (VoiceChatUser && VoiceChatUser->IsLoggedIn())
	{
		VoiceChatUser->SetAudioInputDeviceMuted(bMute);
	}
	else if (VoiceChat && VoiceChat->IsInitialized())
	{
		VoiceChat->SetAudioInputDeviceMuted(bMute);
	}
}

void UEOSVoiceChatSubsystem::SetPTTEnabled(bool bEnabled)
{
	bPTTEnabled = bEnabled;

	if (!bPTTEnabled && !bAlwaysTransmit)
	{
		UpdateRTCSending(false);
	}
	UpdateVoiceIndicatorFromState();
}

void UEOSVoiceChatSubsystem::SetAlwaysTransmit(bool bEnabled)
{
	bAlwaysTransmit = bEnabled;
	bIsPTTActive = false;
	UpdateRTCSending(bAlwaysTransmit);
	UpdateVoiceIndicatorFromState();
}

bool UEOSVoiceChatSubsystem::IsVoiceIndicatorActive() const
{
	if (bAlwaysTransmit)
	{
		return !bMuted;
	}

	if (!bPTTEnabled)
	{
		return false;
	}

	return bIsPTTActive && !bMuted;
}

void UEOSVoiceChatSubsystem::JoinChannel(const FString& ChannelName)
{
	const FString Desired = ChannelName.IsEmpty() ? GetAutoChannelName() : ChannelName;
	const FString Sanitized = SanitizeChannelName(Desired);

	EnsureVoiceChatUser();
	if (!VoiceChatUser || !VoiceChatUser->IsLoggedIn())
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Warning, TEXT("Voice chat is not logged in yet."));
		bPendingAutoJoin = true;
		EnsureAutoJoinTicker();
		return;
	}

	if (bEnableVoiceLobby && bPreferLobbyRtc)
	{
		if (ActiveLobbyId.IsEmpty())
		{
			EnsureVoiceLobby();
			bPendingAutoJoin = true;
			EnsureAutoJoinTicker();
			return;
		}

		if (TryUseLobbyRtc(ActiveLobbyId))
		{
			bPendingAutoJoin = false;
			return;
		}
	}

	if (bPreferLobbyRtc)
	{
		const FString LobbyId = GetLobbyIdFromSession();
		if (!LobbyId.IsEmpty() && TryUseLobbyRtc(LobbyId))
		{
			bPendingAutoJoin = false;
			return;
		}
	}

	if (!ManualClientBaseUrl.IsEmpty() && !ManualParticipantToken.IsEmpty())
	{
		if (TryJoinManualRtc(Sanitized))
		{
			bPendingAutoJoin = false;
			return;
		}
		UE_LOG(LogEOSVoiceChatSubsystem, Warning, TEXT("Manual RTC join failed or missing credentials."));
	}

	if (VoiceChatUser)
	{
		ActiveRoomName = Sanitized;
		ChannelMode = EEOSVoiceChannelMode::VoiceChat;
		VoiceChatUser->JoinChannel(
			Sanitized,
			TEXT(""),
			EVoiceChatChannelType::NonPositional,
			FOnVoiceChatChannelJoinCompleteDelegate::CreateUObject(this, &UEOSVoiceChatSubsystem::HandleVoiceChatChannelJoinComplete),
			TOptional<FVoiceChatChannel3dProperties>());
		bPendingAutoJoin = false;
		return;
	}

	UE_LOG(LogEOSVoiceChatSubsystem, Warning, TEXT("Voice chat user missing. Will retry auto-join."));
	bPendingAutoJoin = true;
	EnsureAutoJoinTicker();
}

void UEOSVoiceChatSubsystem::LeaveChannel()
{
	UpdateRTCSending(false);

	if (ChannelMode == EEOSVoiceChannelMode::VoiceChat)
	{
		if (VoiceChatUser && VoiceChatUser->IsLoggedIn() && !ActiveRoomName.IsEmpty())
		{
			VoiceChatUser->LeaveChannel(
				ActiveRoomName,
				FOnVoiceChatChannelLeaveCompleteDelegate::CreateUObject(this, &UEOSVoiceChatSubsystem::HandleVoiceChatChannelLeaveComplete));
		}

		ChannelMode = EEOSVoiceChannelMode::None;
		ActiveRoomName.Reset();
		bPendingAutoJoin = false;
		return;
	}

	if (ChannelMode == EEOSVoiceChannelMode::LobbyRtc)
	{
#if WITH_EOSVOICECHAT
		if (VoiceChatUser && !ActiveLobbyId.IsEmpty())
		{
			if (FEOSVoiceChatUser* EOSUser = static_cast<FEOSVoiceChatUser*>(VoiceChatUser))
			{
				EOSUser->RemoveLobbyRoom(ActiveLobbyId);
			}
		}
#endif

		ChannelMode = EEOSVoiceChannelMode::None;
		ActiveLobbyId.Reset();
		ActiveRoomName.Reset();
		bPendingAutoJoin = false;
		return;
	}

	if (ChannelMode == EEOSVoiceChannelMode::LobbyRtcSdk)
	{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
		LeaveVoiceLobby();
#endif
		ChannelMode = EEOSVoiceChannelMode::None;
		ActiveLobbyId.Reset();
		ActiveRoomName.Reset();
		bPendingAutoJoin = false;
		return;
	}

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (ChannelMode == EEOSVoiceChannelMode::ManualRtc && EOSRtcHandle && LocalProductUserId && !ActiveRoomName.IsEmpty())
	{
		const FTCHARToUTF8 RoomNameUtf8(*ActiveRoomName);
		EOS_RTC_LeaveRoomOptions LeaveOptions = {};
		LeaveOptions.ApiVersion = EOS_RTC_LEAVEROOM_API_LATEST;
		LeaveOptions.LocalUserId = LocalProductUserId;
		LeaveOptions.RoomName = RoomNameUtf8.Get();

		EOS_RTC_LeaveRoom(EOSRtcHandle, &LeaveOptions, this, &UEOSVoiceChatSubsystem::OnRTCLeaveRoomStatic);
	}

	ChannelMode = EEOSVoiceChannelMode::None;
	ActiveRoomName.Reset();
	ActiveLobbyId.Reset();
	bManualRtcJoined = false;
#endif
}

void UEOSVoiceChatSubsystem::InitializeVoiceChat()
{
	VoiceChat = IVoiceChat::Get();
	if (!VoiceChat)
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("IVoiceChat is not available."));
		return;
	}

	if (!VoiceChat->IsInitialized())
	{
		VoiceChat->Initialize(FOnVoiceChatInitializeCompleteDelegate::CreateUObject(
			this, &UEOSVoiceChatSubsystem::HandleVoiceChatInitialized));
	}
	else
	{
		HandleVoiceChatInitialized(FVoiceChatResult::CreateSuccess());
	}
}

void UEOSVoiceChatSubsystem::HandleVoiceChatInitialized(const FVoiceChatResult& Result)
{
	if (!Result.IsSuccess())
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("Voice chat initialize failed: %s"), *LexToString(Result));
		return;
	}

	bVoiceInitialized = true;
	EnsureVoiceChatUser();

	if (!VoiceChat->IsConnected())
	{
		VoiceChat->Connect(FOnVoiceChatConnectCompleteDelegate::CreateUObject(
			this, &UEOSVoiceChatSubsystem::HandleVoiceChatConnected));
	}
	else
	{
		HandleVoiceChatConnected(FVoiceChatResult::CreateSuccess());
	}
}

void UEOSVoiceChatSubsystem::HandleVoiceChatConnected(const FVoiceChatResult& Result)
{
	if (!Result.IsSuccess())
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("Voice chat connect failed: %s"), *LexToString(Result));
		return;
	}

	bVoiceConnected = true;
	EnsureConnectLogin();
}

void UEOSVoiceChatSubsystem::EnsureConnectLogin()
{
	if (!VoiceChat || !VoiceChat->IsInitialized() || !VoiceChat->IsConnected())
	{
		return;
	}

	if (IsRunningDedicatedServer())
	{
		return;
	}

	EnsureVoiceChatUser();
	if (VoiceChatUser && VoiceChatUser->IsLoggedIn())
	{
		return;
	}

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (TryUseExistingPuid())
	{
		return;
	}

	StartConnectDeviceIdLogin();
#endif
}

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
bool UEOSVoiceChatSubsystem::TryUseExistingPuid()
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!ResolveEOSHandles())
	{
		return false;
	}

	if (LocalProductUserId)
	{
		const FString Puid = ProductUserIdToString(LocalProductUserId);
		if (!Puid.IsEmpty())
		{
			LoginToVoiceChat(Puid);
			return true;
		}
	}

	const int32 LoggedInCount = EOS_Connect_GetLoggedInUsersCount(EOSConnectHandle);
	if (LoggedInCount > 0)
	{
		EOS_ProductUserId UserId = EOS_Connect_GetLoggedInUserByIndex(EOSConnectHandle, 0);
		if (UserId)
		{
			LocalProductUserId = UserId;
			const FString Puid = ProductUserIdToString(UserId);
			if (!Puid.IsEmpty())
			{
				LoginToVoiceChat(Puid);
				return true;
			}
		}
	}
#endif
	return false;
}

void UEOSVoiceChatSubsystem::StartConnectDeviceIdLogin()
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (bConnectLoginInFlight)
	{
		return;
	}

	if (!ResolveEOSHandles())
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("EOS Connect handle not available."));
		return;
	}

	bConnectLoginInFlight = true;

	EOS_Connect_CreateDeviceIdOptions CreateOptions = {};
	CreateOptions.ApiVersion = EOS_CONNECT_CREATEDEVICEID_API_LATEST;
	CreateOptions.DeviceModel = "UE5";

	EOS_Connect_CreateDeviceId(EOSConnectHandle, &CreateOptions, this, &UEOSVoiceChatSubsystem::OnCreateDeviceIdCompleteStatic);
#endif
}

void UEOSVoiceChatSubsystem::HandleCreateDeviceIdComplete(const EOS_Connect_CreateDeviceIdCallbackInfo* Data)
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!Data)
	{
		bConnectLoginInFlight = false;
		return;
	}

	if (Data->ResultCode != EOS_EResult::EOS_Success &&
		Data->ResultCode != EOS_EResult::EOS_DuplicateNotAllowed)
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("CreateDeviceId failed: %s"), EosResultToString(Data->ResultCode));
		bConnectLoginInFlight = false;
		return;
	}

	EOS_Connect_Credentials Credentials = {};
	Credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
	Credentials.Type = EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;
	Credentials.Token = nullptr;

	EOS_Connect_UserLoginInfo LoginInfo = {};
	LoginInfo.ApiVersion = EOS_CONNECT_USERLOGININFO_API_LATEST;
	LoginInfo.DisplayName = "UE5";

	EOS_Connect_LoginOptions LoginOptions = {};
	LoginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
	LoginOptions.Credentials = &Credentials;
	LoginOptions.UserLoginInfo = &LoginInfo;

	EOS_Connect_Login(EOSConnectHandle, &LoginOptions, this, &UEOSVoiceChatSubsystem::OnConnectLoginCompleteStatic);
#endif
}

void UEOSVoiceChatSubsystem::HandleConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data)
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!Data)
	{
		bConnectLoginInFlight = false;
		return;
	}

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		LocalProductUserId = Data->LocalUserId;
		const FString Puid = ProductUserIdToString(LocalProductUserId);
		bConnectLoginInFlight = false;
		if (!Puid.IsEmpty())
		{
			LoginToVoiceChat(Puid);
		}
		return;
	}

	if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
	{
		EOS_Connect_CreateUserOptions CreateUserOptions = {};
		CreateUserOptions.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
		CreateUserOptions.ContinuanceToken = Data->ContinuanceToken;

		EOS_Connect_CreateUser(EOSConnectHandle, &CreateUserOptions, this, &UEOSVoiceChatSubsystem::OnConnectCreateUserCompleteStatic);
		return;
	}

	UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("EOS Connect login failed: %s"), EosResultToString(Data->ResultCode));
	bConnectLoginInFlight = false;
#endif
}

void UEOSVoiceChatSubsystem::HandleConnectCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data)
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!Data)
	{
		bConnectLoginInFlight = false;
		return;
	}

	if (Data->ResultCode != EOS_EResult::EOS_Success)
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("EOS Connect CreateUser failed: %s"), EosResultToString(Data->ResultCode));
		bConnectLoginInFlight = false;
		return;
	}

	LocalProductUserId = Data->LocalUserId;
	const FString Puid = ProductUserIdToString(LocalProductUserId);
	bConnectLoginInFlight = false;
	if (!Puid.IsEmpty())
	{
		LoginToVoiceChat(Puid);
	}
#endif
}

FString UEOSVoiceChatSubsystem::ProductUserIdToString(EOS_ProductUserId UserId) const
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!UserId)
	{
		return FString();
	}

	char Buffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
	int32_t BufferLen = EOS_PRODUCTUSERID_MAX_LENGTH + 1;

	const EOS_EResult Result = EOS_ProductUserId_ToString(UserId, Buffer, &BufferLen);
	if (Result != EOS_EResult::EOS_Success)
	{
		return FString();
	}

	return UTF8_TO_TCHAR(Buffer);
#else
	return FString();
#endif
}

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
void EOS_CALL UEOSVoiceChatSubsystem::OnCreateDeviceIdCompleteStatic(const EOS_Connect_CreateDeviceIdCallbackInfo* Data)
{
	if (!Data)
	{
		return;
	}

	if (UEOSVoiceChatSubsystem* Subsystem = static_cast<UEOSVoiceChatSubsystem*>(Data->ClientData))
	{
		Subsystem->HandleCreateDeviceIdComplete(Data);
	}
}

void EOS_CALL UEOSVoiceChatSubsystem::OnConnectLoginCompleteStatic(const EOS_Connect_LoginCallbackInfo* Data)
{
	if (!Data)
	{
		return;
	}

	if (UEOSVoiceChatSubsystem* Subsystem = static_cast<UEOSVoiceChatSubsystem*>(Data->ClientData))
	{
		Subsystem->HandleConnectLoginComplete(Data);
	}
}

void EOS_CALL UEOSVoiceChatSubsystem::OnConnectCreateUserCompleteStatic(const EOS_Connect_CreateUserCallbackInfo* Data)
{
	if (!Data)
	{
		return;
	}

	if (UEOSVoiceChatSubsystem* Subsystem = static_cast<UEOSVoiceChatSubsystem*>(Data->ClientData))
	{
		Subsystem->HandleConnectCreateUserComplete(Data);
	}
}
#endif

void UEOSVoiceChatSubsystem::LoginToVoiceChat(const FString& ProductUserId)
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (ResolveEOSHandles())
	{
		LocalProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*ProductUserId));
	}
#endif

	if (!VoiceChat || !VoiceChat->IsInitialized() || !VoiceChat->IsConnected())
	{
		return;
	}

	if (VoiceChatUser && VoiceChatUser->IsLoggedIn())
	{
		HandleVoiceChatLoginComplete(ProductUserId, FVoiceChatResult::CreateSuccess());
		return;
	}

	const int32 LocalUserNum = 0;
	const FPlatformUserId PlatformUserId = FPlatformUserId::CreateFromInternalId(LocalUserNum);

	EnsureVoiceChatUser();
	if (!VoiceChatUser)
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("Voice chat user is not available."));
		return;
	}

	VoiceChatUser->Login(
		PlatformUserId,
		ProductUserId,
		TEXT(""),
		FOnVoiceChatLoginCompleteDelegate::CreateUObject(this, &UEOSVoiceChatSubsystem::HandleVoiceChatLoginComplete));
}

#endif

void UEOSVoiceChatSubsystem::HandleVoiceChatLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result)
{
	if (!Result.IsSuccess())
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("Voice chat login failed: %s"), *LexToString(Result));
		return;
	}

	bVoiceLoggedIn = true;
	ApplyDefaultAudioInputDevice();
	LogCurrentInputDevice(TEXT("LoginComplete"));

	UpdateRTCSending(false);

	if (bAutoJoinOnLogin)
	{
		bPendingAutoJoin = true;
		JoinChannel(TEXT(""));
	}
}

void UEOSVoiceChatSubsystem::HandleVoiceChatChannelJoinComplete(const FString& ChannelName, const FVoiceChatResult& Result)
{
	if (!Result.IsSuccess())
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("Voice chat join failed: %s"), *LexToString(Result));
		return;
	}
	UpdateRTCSending(false);
}

void UEOSVoiceChatSubsystem::HandleVoiceChatChannelLeaveComplete(const FString& ChannelName, const FVoiceChatResult& Result)
{
}

void UEOSVoiceChatSubsystem::HandleVoiceChatChannelJoinedLog(const FString& ChannelName)
{
}

void UEOSVoiceChatSubsystem::ApplyDefaultAudioInputDevice()
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!EOSRtcAudioHandle || !LocalProductUserId)
	{
		return;
	}

	EOS_RTCAudio_SetInputDeviceSettingsOptions Options = {};
	Options.ApiVersion = EOS_RTCAUDIO_SETINPUTDEVICESETTINGS_API_LATEST;
	Options.LocalUserId = LocalProductUserId;
	Options.RealDeviceId = nullptr;
	Options.bPlatformAEC = EOS_TRUE;

	EOS_RTCAudio_SetInputDeviceSettings(EOSRtcAudioHandle, &Options, this, &UEOSVoiceChatSubsystem::OnSetInputDeviceSettingsStatic);
#endif
}

void UEOSVoiceChatSubsystem::LogCurrentInputDevice(const TCHAR* Context)
{
	if (!VoiceChatUser)
	{
		return;
	}

	const FVoiceChatDeviceInfo CurrentDevice = VoiceChatUser->GetInputDeviceInfo();
	const FVoiceChatDeviceInfo DefaultDevice = VoiceChatUser->GetDefaultInputDeviceInfo();
}

void UEOSVoiceChatSubsystem::HandleVoiceCaptureLog(const FString& ChannelName, TArrayView<int16> PcmSamples, int SampleRate, int Channels)
{
	if (!bEnableMicActivityLogs)
	{
		return;
	}

	if (PcmSamples.Num() <= 0)
	{
		return;
	}

	int32 MaxAbs = 0;
	for (int16 Sample : PcmSamples)
	{
		const int32 AbsSample = FMath::Abs(static_cast<int32>(Sample));
		if (AbsSample > MaxAbs)
		{
			MaxAbs = AbsSample;
		}
	}

	if (MaxAbs < MicActivityAmplitudeThreshold)
	{
		return;
	}

	const double Now = FPlatformTime::Seconds();
	if ((Now - LastMicLogTime) < MicActivityLogIntervalSeconds)
	{
		return;
	}

	LastMicLogTime = Now;
}

bool UEOSVoiceChatSubsystem::TickBindInput(float DeltaTime)
{
	const bool bInputReady = TryBindInput();
	TryAutoJoin();

	if (bInputReady && !bPendingAutoJoin)
	{
		InputTickHandle.Reset();
		return false;
	}

	return true;
}

bool UEOSVoiceChatSubsystem::TryBindInput()
{
	if (CachedPlayerController.IsValid())
	{
		if (!IsValid(CachedPlayerController->InputComponent))
		{
			UnbindInput();
			CachedPlayerController.Reset();
			return false;
		}

		if (CachedEnhancedInputComponent.IsValid() && !IsValid(CachedEnhancedInputComponent.Get()))
		{
			UnbindInput();
			CachedPlayerController.Reset();
			return false;
		}

		return true;
	}

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return false;
	}

	CachedPlayerController = PC;

	const bool bEnhancedBound = BindEnhancedInput(PC);
	if (bEnhancedBound)
	{
		return true;
	}

	return BindLegacyInput(PC);
}

bool UEOSVoiceChatSubsystem::BindEnhancedInput(APlayerController* PC)
{
	if (!PC)
	{
		return false;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return false;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		return false;
	}

	UInputAction* ActionToBind = nullptr;
	const bool bUseExternalAction = PTTActionAssetPath.IsValid();
	if (bUseExternalAction)
	{
		ActionToBind = Cast<UInputAction>(PTTActionAssetPath.TryLoad());
		if (!ActionToBind)
		{
			UE_LOG(LogEOSVoiceChatSubsystem, Warning, TEXT("Failed to load PTT action asset: %s"), *PTTActionAssetPath.ToString());
			return false;
		}
	}
	else
	{
		if (!PTTAction)
		{
			PTTAction = NewObject<UInputAction>(this);
			PTTAction->ValueType = EInputActionValueType::Boolean;
		}

		if (!PTTMappingContext)
		{
			PTTMappingContext = NewObject<UInputMappingContext>(this);
			PTTMappingContext->MapKey(PTTAction, EKeys::V);
		}

		InputSubsystem->AddMappingContext(PTTMappingContext, MappingPriority);
		ActionToBind = PTTAction;
	}

	UInputComponent* InputComponentToBind = PC->InputComponent;
	if (!IsValid(InputComponentToBind))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			InputComponentToBind = Pawn->InputComponent;
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponentToBind);
	if (!IsValid(EnhancedInputComponent))
	{
		return false;
	}

	if (CachedEnhancedInputComponent.IsValid() && CachedEnhancedInputComponent.Get() != EnhancedInputComponent)
	{
		UnbindInput();
	}

	if (CachedEnhancedInputComponent.IsValid() && EnhancedPressedBindingHandle != 0)
	{
		return true;
	}

	FEnhancedInputActionEventBinding& PressedBinding = EnhancedInputComponent->BindAction(
		ActionToBind, ETriggerEvent::Started, this, &UEOSVoiceChatSubsystem::OnPTTPressed);
	FEnhancedInputActionEventBinding& ReleasedBinding = EnhancedInputComponent->BindAction(
		ActionToBind, ETriggerEvent::Completed, this, &UEOSVoiceChatSubsystem::OnPTTReleased);
	FEnhancedInputActionEventBinding& CanceledBinding = EnhancedInputComponent->BindAction(
		ActionToBind, ETriggerEvent::Canceled, this, &UEOSVoiceChatSubsystem::OnPTTReleased);

	EnhancedPressedBindingHandle = PressedBinding.GetHandle();
	EnhancedReleasedBindingHandle = ReleasedBinding.GetHandle();
	EnhancedCanceledBindingHandle = CanceledBinding.GetHandle();
	CachedEnhancedInputComponent = EnhancedInputComponent;
	return true;
}

bool UEOSVoiceChatSubsystem::BindLegacyInput(APlayerController* PC)
{
	if (!PC)
	{
		return false;
	}

	if (!LegacyInputComponent.IsValid())
	{
		UInputComponent* NewInputComponent = NewObject<UInputComponent>(PC, TEXT("EOSVoiceChatPTTInput"));
		NewInputComponent->RegisterComponent();
		NewInputComponent->bBlockInput = false;
		NewInputComponent->Priority = 10000;

		NewInputComponent->BindKey(EKeys::V, IE_Pressed, this, &UEOSVoiceChatSubsystem::OnPTTPressed);
		NewInputComponent->BindKey(EKeys::V, IE_Released, this, &UEOSVoiceChatSubsystem::OnPTTReleased);

		PC->PushInputComponent(NewInputComponent);
		LegacyInputComponent = NewInputComponent;
	}

	return LegacyInputComponent.IsValid();
}

void UEOSVoiceChatSubsystem::UnbindInput()
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CachedEnhancedInputComponent.Get())
	{
		if (IsValid(EnhancedInputComponent))
		{
			if (EnhancedPressedBindingHandle != 0)
			{
				EnhancedInputComponent->RemoveBindingByHandle(EnhancedPressedBindingHandle);
				EnhancedPressedBindingHandle = 0;
			}

			if (EnhancedReleasedBindingHandle != 0)
			{
				EnhancedInputComponent->RemoveBindingByHandle(EnhancedReleasedBindingHandle);
				EnhancedReleasedBindingHandle = 0;
			}

			if (EnhancedCanceledBindingHandle != 0)
			{
				EnhancedInputComponent->RemoveBindingByHandle(EnhancedCanceledBindingHandle);
				EnhancedCanceledBindingHandle = 0;
			}
		}
	}

	APlayerController* PC = CachedPlayerController.Get();
	if (PC)
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (PTTMappingContext)
				{
					InputSubsystem->RemoveMappingContext(PTTMappingContext);
				}
			}
		}
	}

	CachedEnhancedInputComponent.Reset();

	if (LegacyInputComponent.IsValid())
	{
		if (PC)
		{
			PC->PopInputComponent(LegacyInputComponent.Get());
		}

		LegacyInputComponent->DestroyComponent();
		LegacyInputComponent.Reset();
	}

	CachedPlayerController.Reset();
}

void UEOSVoiceChatSubsystem::OnPTTPressed()
{
	if (bAlwaysTransmit)
	{
		return;
	}

	if (!bPTTEnabled || bMuted)
	{
		return;
	}

	bIsPTTActive = !bIsPTTActive;
	UpdateRTCSending(bIsPTTActive);
	UpdateVoiceIndicatorFromState();
}

void UEOSVoiceChatSubsystem::OnPTTReleased()
{
}

void UEOSVoiceChatSubsystem::UpdateVoiceIndicatorFromState()
{
	const bool bActive = IsVoiceIndicatorActive();
	if (bVoiceIndicatorActive == bActive)
	{
		return;
	}

	bVoiceIndicatorActive = bActive;
	OnVoiceTransmitStateChanged.Broadcast(bVoiceIndicatorActive);
}

void UEOSVoiceChatSubsystem::EnsureVoiceChatUser()
{
	if (!VoiceChat || VoiceChatUser)
	{
		return;
	}

	VoiceChatUser = VoiceChat->CreateUser();
	if (!VoiceChatUser)
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Error, TEXT("Failed to create voice chat user."));
		return;
	}

	if (!ChannelJoinedLogHandle.IsValid())
	{
		ChannelJoinedLogHandle = VoiceChatUser->OnVoiceChatChannelJoined().AddUObject(this, &UEOSVoiceChatSubsystem::HandleVoiceChatChannelJoinedLog);
	}

	if (!VoiceCaptureLogHandle.IsValid())
	{
		VoiceCaptureLogHandle = VoiceChatUser->RegisterOnVoiceChatAfterCaptureAudioReadDelegate(
			FOnVoiceChatAfterCaptureAudioReadDelegate2::FDelegate::CreateUObject(this, &UEOSVoiceChatSubsystem::HandleVoiceCaptureLog));
	}
}

void UEOSVoiceChatSubsystem::EnsureAutoJoinTicker()
{
	if (InputTickHandle.IsValid())
	{
		return;
	}

	InputTickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UEOSVoiceChatSubsystem::TickBindInput),
		0.25f);
}

void UEOSVoiceChatSubsystem::TryAutoJoin()
{
	if (!bPendingAutoJoin)
	{
		return;
	}

	if (!VoiceChatUser || !VoiceChatUser->IsLoggedIn())
	{
		return;
	}

	const double Now = FPlatformTime::Seconds();
	if ((Now - LastAutoJoinAttemptTime) < AutoJoinRetryInterval)
	{
		return;
	}

	LastAutoJoinAttemptTime = Now;

	if (bEnableVoiceLobby && bPreferLobbyRtc)
	{
		if (ActiveLobbyId.IsEmpty())
		{
			EnsureVoiceLobby();
			return;
		}

		if (TryUseLobbyRtc(ActiveLobbyId))
		{
			bPendingAutoJoin = false;
			return;
		}
	}

	JoinChannel(TEXT(""));
}

void UEOSVoiceChatSubsystem::EnsureVoiceLobby()
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!bEnableVoiceLobby)
	{
		return;
	}

	if (bVoiceLobbyInFlight)
	{
		return;
	}

	if (!ResolveEOSHandles() || !EOSLobbyHandle || !LocalProductUserId)
	{
		return;
	}

	if (!ActiveLobbyId.IsEmpty())
	{
		return;
	}

	const FString LobbyIdOverride = BuildVoiceLobbyIdOverride();
	if (LobbyIdOverride.IsEmpty())
	{
		return;
	}

	const FString ServerKey = GetVoiceServerKey();
	VoiceLobbyIdOverride = LobbyIdOverride;
	bVoiceLobbyInFlight = true;
	StartVoiceLobbyJoinById(LobbyIdOverride);
#endif
}

FString UEOSVoiceChatSubsystem::BuildVoiceLobbyIdOverride() const
{
	const FString ServerKey = GetVoiceServerKey();
	if (ServerKey.IsEmpty())
	{
		return FString();
	}

	const uint32 Hash = FCrc::StrCrc32(*ServerKey);
	const FString Base = FString::Printf(TEXT("%s_%08X"), *VoiceLobbyIdPrefix, Hash);
	FString Sanitized = SanitizeChannelName(Base);
	if (Sanitized.Len() > 60)
	{
		Sanitized.LeftInline(60);
	}
	if (Sanitized.Len() < 4)
	{
		Sanitized = TEXT("VOICE_LOBBY");
	}
	return Sanitized;
}

FString UEOSVoiceChatSubsystem::GetVoiceServerKey() const
{
	const FString SessionId = GetLobbyIdFromSession();
	if (!SessionId.IsEmpty())
	{
		return SessionId;
	}

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
	{
		return FString();
	}

	UNetDriver* NetDriver = World->GetNetDriver();
	if (!NetDriver || !NetDriver->ServerConnection)
	{
		return FString();
	}

	return NetDriver->ServerConnection->LowLevelGetRemoteAddress(true);
}

FString UEOSVoiceChatSubsystem::GetLobbyIdFromSession() const
{
	IOnlineSubsystem* OSS = GetEOSSubsystem();
	if (!OSS)
	{
		return FString();
	}

	IOnlineSessionPtr SessionInterface = OSS->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return FString();
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!Session || !Session->SessionInfo.IsValid())
	{
		return FString();
	}

	FString LobbyId;
	if (Session->SessionSettings.Get(TEXT("EOS_LOBBY_ID"), LobbyId) && !LobbyId.IsEmpty())
	{
		return LobbyId;
	}

	return Session->SessionInfo->GetSessionId().ToString();
}

FString UEOSVoiceChatSubsystem::GetAutoChannelName() const
{
	const FString SessionId = GetLobbyIdFromSession();
	FString Base;
	if (!SessionId.IsEmpty())
	{
		Base = SessionId;
	}
	else if (!ManualChannelName.IsEmpty())
	{
		Base = ManualChannelName;
	}
	else
	{
		Base = FallbackChannelName;
	}
	const FString Chosen = Base.IsEmpty() ? TEXT("eos_voice_default") : Base;
	return SanitizeChannelName(Chosen);
}

FString UEOSVoiceChatSubsystem::SanitizeChannelName(const FString& InName)
{
	FString Out;
	Out.Reserve(InName.Len());

	for (TCHAR C : InName)
	{
		if (FChar::IsAlnum(C))
		{
			Out.AppendChar(C);
		}
		else
		{
			Out.AppendChar(TEXT('_'));
		}
	}

	if (Out.Len() > 64)
	{
		Out.LeftInline(64);
	}

	if (Out.IsEmpty())
	{
		return TEXT("eos_voice_default");
	}

	return Out;
}

bool UEOSVoiceChatSubsystem::TryUseLobbyRtc(const FString& LobbyId)
{
#if WITH_EOSVOICECHAT
	if (!VoiceChatUser || !VoiceChatUser->IsLoggedIn())
	{
		return false;
	}

	if (LobbyId.IsEmpty())
	{
		return false;
	}

	FEOSVoiceChatUser* EOSUser = static_cast<FEOSVoiceChatUser*>(VoiceChatUser);
	if (!EOSUser)
	{
		return false;
	}

	if (!EOSUser->AddLobbyRoom(LobbyId))
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Warning, TEXT("AddLobbyRoom failed for LobbyId: %s"), *LobbyId);
	}
	else
	{
		ActiveLobbyId = LobbyId;
		ChannelMode = EEOSVoiceChannelMode::LobbyRtc;
		UpdateRTCSending(false);
		return true;
	}

#endif

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!EOSLobbyHandle || !LocalProductUserId)
	{
		return false;
	}

	const FTCHARToUTF8 LobbyIdUtf8(*LobbyId);
	EOS_Lobby_GetRTCRoomNameOptions GetRoomNameOptions = {};
	GetRoomNameOptions.ApiVersion = EOS_LOBBY_GETRTCROOMNAME_API_LATEST;
	GetRoomNameOptions.LobbyId = LobbyIdUtf8.Get();
	GetRoomNameOptions.LocalUserId = LocalProductUserId;
	char Utf8RoomName[256];
	uint32_t Utf8RoomNameLength = UE_ARRAY_COUNT(Utf8RoomName);
	EOS_EResult RoomResult = EOS_Lobby_GetRTCRoomName(EOSLobbyHandle, &GetRoomNameOptions, Utf8RoomName, &Utf8RoomNameLength);
	if (RoomResult != EOS_EResult::EOS_Success)
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Warning, TEXT("EOS_Lobby_GetRTCRoomName failed: %s"), EosResultToString(RoomResult));
		return false;
	}

	const FString RoomName = UTF8_TO_TCHAR(Utf8RoomName);
	ActiveLobbyId = LobbyId;
	ActiveRoomName = RoomName;
	ChannelMode = EEOSVoiceChannelMode::LobbyRtcSdk;
	UpdateRTCSending(false);
	RegisterRtcAudioNotify();
	return true;
#else
	return false;
#endif
}

bool UEOSVoiceChatSubsystem::TryJoinManualRtc(const FString& ChannelName)
{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!EOSRtcHandle || !LocalProductUserId)
	{
		return false;
	}

	if (ManualClientBaseUrl.IsEmpty() || ManualParticipantToken.IsEmpty())
	{
		UE_LOG(LogEOSVoiceChatSubsystem, Warning, TEXT("Manual RTC credentials are missing."));
		return false;
	}

	const FString DesiredRoom = ChannelName.IsEmpty() ? GetAutoChannelName() : ChannelName;

	const FTCHARToUTF8 RoomNameUtf8(*DesiredRoom);
	const FTCHARToUTF8 ClientBaseUrlUtf8(*ManualClientBaseUrl);
	const FTCHARToUTF8 ParticipantTokenUtf8(*ManualParticipantToken);

	EOS_RTC_JoinRoomOptions JoinOptions = {};
	JoinOptions.ApiVersion = EOS_RTC_JOINROOM_API_LATEST;
	JoinOptions.LocalUserId = LocalProductUserId;
	JoinOptions.RoomName = RoomNameUtf8.Get();
	JoinOptions.ClientBaseUrl = ClientBaseUrlUtf8.Get();
	JoinOptions.ParticipantId = LocalProductUserId;
	JoinOptions.ParticipantToken = ParticipantTokenUtf8.Get();

	EOS_RTC_JoinRoom(EOSRtcHandle, &JoinOptions, this, &UEOSVoiceChatSubsystem::OnRTCJoinRoomStatic);

	ActiveRoomName = DesiredRoom;
	ChannelMode = EEOSVoiceChannelMode::ManualRtc;
	RegisterRtcAudioNotify();

	return true;
#else
	return false;
#endif
}

void UEOSVoiceChatSubsystem::UpdateRTCSending(bool bEnable)
{
	const bool bShouldSend = bAlwaysTransmit ? !bMuted : (bEnable && !bMuted);

	if (ChannelMode == EEOSVoiceChannelMode::VoiceChat)
	{
		if (!VoiceChatUser || !VoiceChatUser->IsLoggedIn() || ActiveRoomName.IsEmpty())
		{
			return;
		}

		if (!bShouldSend)
		{
			VoiceChatUser->TransmitToNoChannels();
			return;
		}

		TSet<FString> Channels;
		Channels.Add(ActiveRoomName);
		VoiceChatUser->TransmitToSpecificChannels(Channels);
		return;
	}

	if (ChannelMode == EEOSVoiceChannelMode::LobbyRtc)
	{
		if (!VoiceChatUser || !VoiceChatUser->IsLoggedIn())
		{
			return;
		}

		if (!bShouldSend)
		{
			VoiceChatUser->TransmitToNoChannels();
			return;
		}

		VoiceChatUser->TransmitToAllChannels();
		return;
	}

	if (ChannelMode == EEOSVoiceChannelMode::LobbyRtcSdk)
	{
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
		if (!EOSRtcAudioHandle || !LocalProductUserId || ActiveRoomName.IsEmpty())
		{
			return;
		}

		if (!bShouldSend)
		{
			EOS_RTCAudio_UpdateSendingOptions Options = {};
			Options.ApiVersion = EOS_RTCAUDIO_UPDATESENDING_API_LATEST;
			Options.LocalUserId = LocalProductUserId;
			Options.RoomName = TCHAR_TO_UTF8(*ActiveRoomName);
			Options.AudioStatus = EOS_ERTCAudioStatus::EOS_RTCAS_Disabled;
			EOS_RTCAudio_UpdateSending(EOSRtcAudioHandle, &Options, this, &UEOSVoiceChatSubsystem::OnRTCSendUpdateStatic);
			return;
		}

		EOS_RTCAudio_UpdateSendingOptions Options = {};
		Options.ApiVersion = EOS_RTCAUDIO_UPDATESENDING_API_LATEST;
		Options.LocalUserId = LocalProductUserId;
		Options.RoomName = TCHAR_TO_UTF8(*ActiveRoomName);
		Options.AudioStatus = EOS_ERTCAudioStatus::EOS_RTCAS_Enabled;
		EOS_RTCAudio_UpdateSending(EOSRtcAudioHandle, &Options, this, &UEOSVoiceChatSubsystem::OnRTCSendUpdateStatic);
#endif
		return;
	}

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	if (!EOSRtcAudioHandle || !LocalProductUserId || ActiveRoomName.IsEmpty())
	{
		return;
	}

	const FTCHARToUTF8 RoomNameUtf8(*ActiveRoomName);

	EOS_RTCAudio_UpdateSendingOptions Options = {};
	Options.ApiVersion = EOS_RTCAUDIO_UPDATESENDING_API_LATEST;
	Options.LocalUserId = LocalProductUserId;
	Options.RoomName = RoomNameUtf8.Get();
	Options.AudioStatus = bShouldSend ? EOS_ERTCAudioStatus::EOS_RTCAS_Enabled : EOS_ERTCAudioStatus::EOS_RTCAS_Disabled;

	EOS_RTCAudio_UpdateSending(EOSRtcAudioHandle, &Options, this, &UEOSVoiceChatSubsystem::OnRTCSendUpdateStatic);
#endif
}

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
void EOS_CALL UEOSVoiceChatSubsystem::OnRTCSendUpdateStatic(const EOS_RTCAudio_UpdateSendingCallbackInfo* Data)
{
	if (!Data)
	{
		return;
	}
}

void EOS_CALL UEOSVoiceChatSubsystem::OnRTCJoinRoomStatic(const EOS_RTC_JoinRoomCallbackInfo* Data)
{
	if (!Data)
	{
		return;
	}

	const FString RoomName = Data->RoomName ? UTF8_TO_TCHAR(Data->RoomName) : TEXT("");
	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		if (Data->ClientData)
		{
			static_cast<UEOSVoiceChatSubsystem*>(Data->ClientData)->ApplyDefaultAudioInputDevice();
		}
	}
	else
	{
	}
}

void EOS_CALL UEOSVoiceChatSubsystem::OnRTCLeaveRoomStatic(const EOS_RTC_LeaveRoomCallbackInfo* Data)
{
	if (!Data)
	{
		return;
	}
}

void EOS_CALL UEOSVoiceChatSubsystem::OnRTCAudioBeforeSendStatic(const EOS_RTCAudio_AudioBeforeSendCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	static_cast<UEOSVoiceChatSubsystem*>(Data->ClientData)->HandleRtcAudioBeforeSend(Data);
}

void EOS_CALL UEOSVoiceChatSubsystem::OnSetInputDeviceSettingsStatic(const EOS_RTCAudio_OnSetInputDeviceSettingsCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	static_cast<UEOSVoiceChatSubsystem*>(Data->ClientData)->HandleSetInputDeviceSettings(Data);
}

void EOS_CALL UEOSVoiceChatSubsystem::OnLobbyJoinByIdCompleteStatic(const EOS_Lobby_JoinLobbyByIdCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	static_cast<UEOSVoiceChatSubsystem*>(Data->ClientData)->HandleVoiceLobbyJoinByIdComplete(Data);
}

void EOS_CALL UEOSVoiceChatSubsystem::OnLobbyCreateCompleteStatic(const EOS_Lobby_CreateLobbyCallbackInfo* Data)
{
	if (!Data || !Data->ClientData)
	{
		return;
	}

	static_cast<UEOSVoiceChatSubsystem*>(Data->ClientData)->HandleVoiceLobbyCreateComplete(Data);
}

void UEOSVoiceChatSubsystem::StartVoiceLobbyJoinById(const FString& LobbyIdOverride)
{
	if (!EOSLobbyHandle || !LocalProductUserId)
	{
		bVoiceLobbyInFlight = false;
		return;
	}

	const FTCHARToUTF8 LobbyIdUtf8(*LobbyIdOverride);
	EOS_Lobby_JoinLobbyByIdOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_JOINLOBBYBYID_API_LATEST;
	Options.LobbyId = LobbyIdUtf8.Get();
	Options.LocalUserId = LocalProductUserId;
	Options.bPresenceEnabled = EOS_FALSE;
	Options.LocalRTCOptions = nullptr;
	Options.bCrossplayOptOut = EOS_FALSE;
	Options.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin;
	EOS_Lobby_JoinLobbyById(EOSLobbyHandle, &Options, this, &UEOSVoiceChatSubsystem::OnLobbyJoinByIdCompleteStatic);
}

void UEOSVoiceChatSubsystem::StartVoiceLobbyCreate(const FString& LobbyIdOverride)
{
	if (!EOSLobbyHandle || !LocalProductUserId)
	{
		bVoiceLobbyInFlight = false;
		return;
	}

	const FTCHARToUTF8 LobbyIdUtf8(*LobbyIdOverride);
	const FTCHARToUTF8 BucketUtf8(*VoiceLobbyBucketId);

	EOS_Lobby_CreateLobbyOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
	Options.LocalUserId = LocalProductUserId;
	Options.MaxLobbyMembers = VoiceLobbyMaxMembers > 0 ? static_cast<uint32_t>(VoiceLobbyMaxMembers) : 16u;
	Options.PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
	Options.bPresenceEnabled = EOS_FALSE;
	Options.bAllowInvites = EOS_FALSE;
	Options.BucketId = BucketUtf8.Get();
	Options.bDisableHostMigration = EOS_FALSE;
	Options.bEnableRTCRoom = EOS_TRUE;
	Options.LocalRTCOptions = nullptr;
	Options.LobbyId = LobbyIdUtf8.Get();
	Options.bEnableJoinById = EOS_TRUE;
	Options.bRejoinAfterKickRequiresInvite = EOS_FALSE;
	Options.AllowedPlatformIds = nullptr;
	Options.AllowedPlatformIdsCount = 0;
	Options.bCrossplayOptOut = EOS_FALSE;
	Options.RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin;
	EOS_Lobby_CreateLobby(EOSLobbyHandle, &Options, this, &UEOSVoiceChatSubsystem::OnLobbyCreateCompleteStatic);
}

void UEOSVoiceChatSubsystem::HandleVoiceLobbyJoinByIdComplete(const EOS_Lobby_JoinLobbyByIdCallbackInfo* Data)
{
	if (!Data)
	{
		bVoiceLobbyInFlight = false;
		return;
	}

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		const FString LobbyId = UTF8_TO_TCHAR(Data->LobbyId);
		HandleVoiceLobbyReady(LobbyId);
		return;
	}

	if (Data->ResultCode == EOS_EResult::EOS_NotFound)
	{
		StartVoiceLobbyCreate(VoiceLobbyIdOverride);
		return;
	}

	bVoiceLobbyInFlight = false;
	UE_LOG(LogEOSVoiceChatSubsystem, Warning, TEXT("Voice lobby join by id failed: %s"), EosResultToString(Data->ResultCode));
	bPendingAutoJoin = true;
	EnsureAutoJoinTicker();
}

void UEOSVoiceChatSubsystem::HandleVoiceLobbyCreateComplete(const EOS_Lobby_CreateLobbyCallbackInfo* Data)
{
	if (!Data)
	{
		bVoiceLobbyInFlight = false;
		return;
	}

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		const FString LobbyId = UTF8_TO_TCHAR(Data->LobbyId);
		HandleVoiceLobbyReady(LobbyId);
		return;
	}

	bVoiceLobbyInFlight = false;
	UE_LOG(LogEOSVoiceChatSubsystem, Warning, TEXT("Voice lobby create failed: %s"), EosResultToString(Data->ResultCode));
	bPendingAutoJoin = true;
	EnsureAutoJoinTicker();
}

void UEOSVoiceChatSubsystem::HandleVoiceLobbyReady(const FString& LobbyId)
{
	bVoiceLobbyInFlight = false;
	ActiveLobbyId = LobbyId;
	if (TryUseLobbyRtc(ActiveLobbyId))
	{
		bPendingAutoJoin = false;
		return;
	}

	bPendingAutoJoin = true;
	EnsureAutoJoinTicker();
}

void UEOSVoiceChatSubsystem::LeaveVoiceLobby()
{
	if (!EOSLobbyHandle || !LocalProductUserId || ActiveLobbyId.IsEmpty())
	{
		ActiveLobbyId.Reset();
		VoiceLobbyIdOverride.Reset();
		bVoiceLobbyInFlight = false;
		return;
	}

	UnregisterRtcAudioNotify();

	const FTCHARToUTF8 LobbyIdUtf8(*ActiveLobbyId);
	EOS_Lobby_LeaveLobbyOptions Options = {};
	Options.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
	Options.LocalUserId = LocalProductUserId;
	Options.LobbyId = LobbyIdUtf8.Get();

	EOS_Lobby_LeaveLobby(EOSLobbyHandle, &Options, nullptr, nullptr);
	ActiveLobbyId.Reset();
	VoiceLobbyIdOverride.Reset();
	bVoiceLobbyInFlight = false;
}

void UEOSVoiceChatSubsystem::RegisterRtcAudioNotify()
{
	if (!bEnableMicActivityLogs)
	{
		return;
	}

	if (!EOSRtcAudioHandle || !LocalProductUserId || ActiveRoomName.IsEmpty())
	{
		return;
	}

	if (RtcBeforeSendNotifyId != 0 && RtcBeforeSendRoomName == ActiveRoomName)
	{
		return;
	}

	UnregisterRtcAudioNotify();

	EOS_RTCAudio_AddNotifyAudioBeforeSendOptions Options = {};
	Options.ApiVersion = EOS_RTCAUDIO_ADDNOTIFYAUDIOBEFORESEND_API_LATEST;
	Options.LocalUserId = LocalProductUserId;
	Options.RoomName = TCHAR_TO_UTF8(*ActiveRoomName);

	RtcBeforeSendNotifyId = EOS_RTCAudio_AddNotifyAudioBeforeSend(EOSRtcAudioHandle, &Options, this, &UEOSVoiceChatSubsystem::OnRTCAudioBeforeSendStatic);
	RtcBeforeSendRoomName = ActiveRoomName;
}

void UEOSVoiceChatSubsystem::UnregisterRtcAudioNotify()
{
	if (!EOSRtcAudioHandle || RtcBeforeSendNotifyId == 0)
	{
		RtcBeforeSendRoomName.Reset();
		return;
	}

	EOS_RTCAudio_RemoveNotifyAudioBeforeSend(EOSRtcAudioHandle, RtcBeforeSendNotifyId);
	RtcBeforeSendNotifyId = 0;
	RtcBeforeSendRoomName.Reset();
}

void UEOSVoiceChatSubsystem::HandleRtcAudioBeforeSend(const EOS_RTCAudio_AudioBeforeSendCallbackInfo* Data)
{
	if (!Data || !Data->Buffer || !bEnableMicActivityLogs)
	{
		return;
	}

	const EOS_RTCAudio_AudioBuffer* Buffer = Data->Buffer;
	if (!Buffer->Frames || Buffer->FramesCount == 0 || Buffer->Channels == 0)
	{
		return;
	}

	const uint32 SamplesCount = Buffer->FramesCount * Buffer->Channels;
	int32 MaxAbs = 0;
	for (uint32 Index = 0; Index < SamplesCount; ++Index)
	{
		const int32 AbsSample = FMath::Abs(static_cast<int32>(Buffer->Frames[Index]));
		if (AbsSample > MaxAbs)
		{
			MaxAbs = AbsSample;
		}
	}

	if (MaxAbs < MicActivityAmplitudeThreshold)
	{
		return;
	}

	const double Now = FPlatformTime::Seconds();
	if ((Now - LastMicLogTime) < MicActivityLogIntervalSeconds)
	{
		return;
	}

	LastMicLogTime = Now;

	const FString RoomName = Data->RoomName ? UTF8_TO_TCHAR(Data->RoomName) : TEXT("");
}

void UEOSVoiceChatSubsystem::HandleSetInputDeviceSettings(const EOS_RTCAudio_OnSetInputDeviceSettingsCallbackInfo* Data)
{
	if (!Data)
	{
		return;
	}
	LogCurrentInputDevice(TEXT("RtcSetInputDevice"));
}

bool UEOSVoiceChatSubsystem::ResolveEOSHandles()
{
	if (EOSPlatformHandle.IsValid())
	{
		return true;
	}

	IOnlineSubsystem* OSS = GetEOSSubsystem();
	IOnlineSubsystemEOS* EOSOSS = nullptr;
	if (OSS && OSS->GetSubsystemName() == TEXT("EOS"))
	{
		EOSOSS = static_cast<IOnlineSubsystemEOS*>(OSS);
	}
	if (!EOSOSS)
	{
		return false;
	}

	EOSPlatformHandle = EOSOSS->GetEOSPlatformHandle();
	if (!EOSPlatformHandle.IsValid())
	{
		return false;
	}

	EOS_HPlatform PlatformHandle = EOSPlatformHandle.Get()->operator EOS_HPlatform();
	EOSConnectHandle = EOS_Platform_GetConnectInterface(PlatformHandle);
	EOSLobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);
	EOSRtcHandle = EOS_Platform_GetRTCInterface(PlatformHandle);
	EOSRtcAudioHandle = EOS_RTC_GetAudioInterface(EOSRtcHandle);

	return EOSConnectHandle && EOSLobbyHandle && EOSRtcHandle && EOSRtcAudioHandle;
}

EOS_ProductUserId UEOSVoiceChatSubsystem::GetLocalProductUserId() const
{
	return LocalProductUserId;
}
#endif

IOnlineSubsystem* UEOSVoiceChatSubsystem::GetEOSSubsystem() const
{
	if (IOnlineSubsystem* EOS = IOnlineSubsystem::Get(TEXT("EOS")))
	{
		return EOS;
	}
	return IOnlineSubsystem::Get();
}
