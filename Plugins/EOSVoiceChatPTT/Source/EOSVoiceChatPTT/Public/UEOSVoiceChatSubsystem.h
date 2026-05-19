#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VoiceChatResult.h"
#include "Containers/Ticker.h"
#include "UObject/SoftObjectPath.h"
#include "UEOSVoiceChatSubsystem.generated.h"

class UInputAction;
class UInputMappingContext;
class UInputComponent;
class UEnhancedInputComponent;
class IVoiceChat;
class IVoiceChatUser;
class IOnlineSubsystem;
class APlayerController;
class IInputProcessor;

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
#include "eos_common.h"
#include "eos_connect.h"
#include "eos_lobby.h"
#include "eos_rtc.h"
#include "eos_rtc_audio.h"
#endif

UENUM()
enum class EEOSVoiceChannelMode : uint8
{
	None,
	VoiceChat,
	LobbyRtc,
	LobbyRtcSdk,
	ManualRtc
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceTransmitStateChanged, bool, bIsTransmitting);

UCLASS(Config=Engine)
class EOSVOICECHATPTT_API UEOSVoiceChatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEOSVoiceChatSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="EOS Voice Chat")
	void ToggleMute(bool bMute);

	UFUNCTION(BlueprintCallable, Category="EOS Voice Chat")
	void SetPTTEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="EOS Voice Chat")
	void SetAlwaysTransmit(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="EOS Voice Chat")
	void JoinChannel(const FString& ChannelName);

	UFUNCTION(BlueprintCallable, Category="EOS Voice Chat")
	void LeaveChannel();

	UFUNCTION(BlueprintCallable, Category="EOS Voice Chat")
	bool IsVoiceIndicatorActive() const;

	UPROPERTY(BlueprintAssignable, Category="EOS Voice Chat")
	FOnVoiceTransmitStateChanged OnVoiceTransmitStateChanged;

private:
	void InitializeVoiceChat();
	void HandleVoiceChatInitialized(const FVoiceChatResult& Result);
	void HandleVoiceChatConnected(const FVoiceChatResult& Result);
	void HandleVoiceChatLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result);
	void HandleVoiceChatChannelJoinComplete(const FString& ChannelName, const FVoiceChatResult& Result);
	void HandleVoiceChatChannelLeaveComplete(const FString& ChannelName, const FVoiceChatResult& Result);
	void HandleVoiceChatChannelJoinedLog(const FString& ChannelName);
	void HandleVoiceCaptureLog(const FString& ChannelName, TArrayView<int16> PcmSamples, int SampleRate, int Channels);
	void ApplyDefaultAudioInputDevice();
	void LogCurrentInputDevice(const TCHAR* Context);

	void EnsureVoiceChatUser();
	void EnsureAutoJoinTicker();
	void TryAutoJoin();
	void EnsureVoiceLobby();
	FString BuildVoiceLobbyIdOverride() const;
	FString GetVoiceServerKey() const;

	void EnsureConnectLogin();
#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	bool TryUseExistingPuid();
	void StartConnectDeviceIdLogin();
	void HandleCreateDeviceIdComplete(const EOS_Connect_CreateDeviceIdCallbackInfo* Data);
	void HandleConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data);
	void HandleConnectCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data);
	FString ProductUserIdToString(EOS_ProductUserId UserId) const;
	void LoginToVoiceChat(const FString& ProductUserId);
#endif

	bool TickBindInput(float DeltaTime);
	bool TryBindInput();
	bool BindEnhancedInput(APlayerController* PC);
	bool BindLegacyInput(APlayerController* PC);
	void UnbindInput();

	void OnPTTPressed();
	void OnPTTReleased();

	FString GetLobbyIdFromSession() const;
	FString GetAutoChannelName() const;
	static FString SanitizeChannelName(const FString& InName);

	bool TryUseLobbyRtc(const FString& LobbyId);
	bool TryJoinManualRtc(const FString& ChannelName);

	void UpdateRTCSending(bool bEnable);
	void UpdateVoiceIndicatorFromState();

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	static void EOS_CALL OnRTCSendUpdateStatic(const EOS_RTCAudio_UpdateSendingCallbackInfo* Data);
	static void EOS_CALL OnRTCJoinRoomStatic(const EOS_RTC_JoinRoomCallbackInfo* Data);
	static void EOS_CALL OnRTCLeaveRoomStatic(const EOS_RTC_LeaveRoomCallbackInfo* Data);
	static void EOS_CALL OnRTCAudioBeforeSendStatic(const EOS_RTCAudio_AudioBeforeSendCallbackInfo* Data);
	static void EOS_CALL OnSetInputDeviceSettingsStatic(const EOS_RTCAudio_OnSetInputDeviceSettingsCallbackInfo* Data);
	static void EOS_CALL OnCreateDeviceIdCompleteStatic(const EOS_Connect_CreateDeviceIdCallbackInfo* Data);
	static void EOS_CALL OnConnectLoginCompleteStatic(const EOS_Connect_LoginCallbackInfo* Data);
	static void EOS_CALL OnConnectCreateUserCompleteStatic(const EOS_Connect_CreateUserCallbackInfo* Data);
	static void EOS_CALL OnLobbyJoinByIdCompleteStatic(const EOS_Lobby_JoinLobbyByIdCallbackInfo* Data);
	static void EOS_CALL OnLobbyCreateCompleteStatic(const EOS_Lobby_CreateLobbyCallbackInfo* Data);

	bool ResolveEOSHandles();
	EOS_ProductUserId GetLocalProductUserId() const;
	void StartVoiceLobbyJoinById(const FString& LobbyIdOverride);
	void StartVoiceLobbyCreate(const FString& LobbyIdOverride);
	void HandleVoiceLobbyJoinByIdComplete(const EOS_Lobby_JoinLobbyByIdCallbackInfo* Data);
	void HandleVoiceLobbyCreateComplete(const EOS_Lobby_CreateLobbyCallbackInfo* Data);
	void HandleVoiceLobbyReady(const FString& LobbyId);
	void LeaveVoiceLobby();
	void RegisterRtcAudioNotify();
	void UnregisterRtcAudioNotify();
	void HandleRtcAudioBeforeSend(const EOS_RTCAudio_AudioBeforeSendCallbackInfo* Data);
	void HandleSetInputDeviceSettings(const EOS_RTCAudio_OnSetInputDeviceSettingsCallbackInfo* Data);
#endif

	IOnlineSubsystem* GetEOSSubsystem() const;

private:
	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	FString FallbackChannelName;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	bool bAutoJoinOnLogin;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	bool bEnablePTTByDefault;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	bool bAlwaysTransmitByDefault;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	int32 MappingPriority;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	bool bPreferLobbyRtc;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	bool bEnableVoiceLobby;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	FString VoiceLobbyIdPrefix;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	FString VoiceLobbyBucketId;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	int32 VoiceLobbyMaxMembers;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	FSoftObjectPath PTTActionAssetPath;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	FString AutoJoinMapName;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	FString ManualClientBaseUrl;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	FString ManualParticipantToken;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	FString ManualChannelName;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	bool bEnableMicActivityLogs;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	float MicActivityLogIntervalSeconds;

	UPROPERTY(Config, EditDefaultsOnly, Category="EOS Voice Chat")
	int32 MicActivityAmplitudeThreshold;

	IVoiceChat* VoiceChat;
	IVoiceChatUser* VoiceChatUser;

	bool bVoiceInitialized;
	bool bVoiceConnected;
	bool bVoiceLoggedIn;
	bool bPTTEnabled;
	bool bAlwaysTransmit;
	bool bMuted;
	bool bIsPTTActive;
	bool bVoiceIndicatorActive;
	bool bPendingAutoJoin;
	bool bVoiceLobbyInFlight;

	double LastAutoJoinAttemptTime;
	double LastMicLogTime;
	float AutoJoinRetryInterval;

	EEOSVoiceChannelMode ChannelMode;
	FString ActiveRoomName;
	FString ActiveLobbyId;

	FTSTicker::FDelegateHandle InputTickHandle;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> PTTMappingContext;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> PTTAction;

	uint32 EnhancedPressedBindingHandle;
	uint32 EnhancedReleasedBindingHandle;
	uint32 EnhancedCanceledBindingHandle;

	TWeakObjectPtr<UInputComponent> LegacyInputComponent;
	TWeakObjectPtr<UEnhancedInputComponent> CachedEnhancedInputComponent;
	TWeakObjectPtr<APlayerController> CachedPlayerController;
	TSharedPtr<IInputProcessor> PTTInputPreProcessor;
	FDelegateHandle MapLoadedHandle;
	FDelegateHandle VoiceCaptureLogHandle;
	FDelegateHandle ChannelJoinedLogHandle;

#if defined(WITH_EOS_SDK) && WITH_EOS_SDK
	TSharedPtr<class IEOSPlatformHandle, ESPMode::ThreadSafe> EOSPlatformHandle;
	EOS_HConnect EOSConnectHandle;
	EOS_HLobby EOSLobbyHandle;
	EOS_HRTC EOSRtcHandle;
	EOS_HRTCAudio EOSRtcAudioHandle;
	EOS_ProductUserId LocalProductUserId;
	bool bManualRtcJoined;
	bool bConnectLoginInFlight;
	FString VoiceLobbyIdOverride;
	uint64 RtcBeforeSendNotifyId;
	FString RtcBeforeSendRoomName;
#endif
};
