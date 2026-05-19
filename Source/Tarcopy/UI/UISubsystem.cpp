// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "UI/UW_RootHUD.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UEOSVoiceChatSubsystem.h"
#include "Inventory/InventoryData.h"
#include "Misc/Guid.h"
#include "UI/Inventory/UW_Inventory.h"
#include "UI/Inventory/UW_InventoryBorder.h"
#include "Item/ItemInstance.h"
#include "UI/Inventory/UW_ItemCommandMenu.h"
#include "Car/TCCarBase.h"
#include "Car/UI/TCCarActivate.h"

UUISubsystem::UUISubsystem()
{
    static ConstructorHelpers::FObjectFinder<UUIConfig> ConfigObj(TEXT("/Game/Tarcopy/Main/Blueprints/UI/DA_UIConfig.DA_UIConfig"));
    if (ConfigObj.Succeeded())
    {
        UIConfigData = ConfigObj.Object;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem(): Failed to find UIConfigData asset!"));
    }
}

void UUISubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
    Super::PlayerControllerChanged(NewPlayerController);

    ResetAllUI();
    InitRootHUD();
    BindVoiceIndicator();
}

UUserWidget* UUISubsystem::ShowUI(EUIType Type)
{
    // 이미 생성된 UI가 있을 경우
    if (TObjectPtr<UUserWidget>* Found = SingleWidgets.Find(Type))
    {
        (*Found)->SetVisibility(ESlateVisibility::Visible);
        return Found->Get();
    }
    
    // UI를 생성해야 하는 경우
    APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: PC is nullptr!"));
        return nullptr;
    }

    FUIInfo WidgetInfo;
    if (!UIConfigData->GetInfo(Type, WidgetInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: Cannot Find UIConfig Info!"));
        return nullptr;
    }
    
    UUserWidget* WidgetInstance = CreateWidget<UUserWidget>(PC, WidgetInfo.WidgetClass);
    if (!WidgetInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: WidgetInstance is nullptr!"));
        return nullptr;
    }

    if (!RootHUD)
    {
        InitRootHUD();
    }
    UCanvasPanelSlot* Slot = RootHUD->GetRootCanvas()->AddChildToCanvas(WidgetInstance);
    if (!Slot)
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: Slot is nullptr!"));
        return nullptr;
    }
    ApplyLayoutPreset(Slot, WidgetInfo.Layout);

    SingleWidgets.Add(Type, WidgetInstance);

    return WidgetInstance;
}

void UUISubsystem::HideUI(EUIType Type)
{
    if (TObjectPtr<UUserWidget>* Found = SingleWidgets.Find(Type))
    {
        (*Found)->SetVisibility(ESlateVisibility::Collapsed);
    }
}

UUW_InventoryBorder* UUISubsystem::ShowInventoryUI(UInventoryData* InventoryData)
{
    if (!IsValid(InventoryData))
    {
        return nullptr;
    }

    for (auto It = InventoryWidgets.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid() || !IsValid(It.Value()))
        {
            It.RemoveCurrent();
        }
    }

    // 이미 떠 있으면 재사용
    if (TObjectPtr<UUW_InventoryBorder>* Found = InventoryWidgets.Find(InventoryData))
    {
        if (IsValid(*Found))
        {
            (*Found)->SetVisibility(ESlateVisibility::Visible);

            InventoryData->ForceRefreshNextTick();

            return Found->Get();
        }
        InventoryWidgets.Remove(InventoryData);
    }

    // UI를 생성해야 하는 경우
    APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: PC is nullptr!"));
        return nullptr;
    }

    FUIInfo WidgetInfo;
    if (!UIConfigData->GetInfo(EUIType::Inventory, WidgetInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: Cannot Find UIConfig Info!"));
        return nullptr;
    }

    UUW_Inventory* InventoryInstance = CreateWidget<UUW_Inventory>(PC, WidgetInfo.WidgetClass);
    if (!InventoryInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: InventoryInstance is nullptr!"));
        return nullptr;
    }

    InventoryInstance->BindInventory(InventoryData);

    InventoryData->ForceRefreshNextTick();

    FUIInfo BorderWidgetInfo;
    if (!UIConfigData->GetInfo(EUIType::InventoryBorder, BorderWidgetInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: Cannot Find UIConfig Info!"));
        return nullptr;
    }

    UUW_InventoryBorder* BorderInstance = CreateWidget<UUW_InventoryBorder>(PC, BorderWidgetInfo.WidgetClass);
    if (!BorderInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: BorderInstance is nullptr!"));
        return nullptr;
    }

    BorderInstance->SetContentWidget(InventoryInstance);
    BorderInstance->SetInventoryData(InventoryData);

    if (!RootHUD)
    {
        InitRootHUD();
    }
    UCanvasPanelSlot* Slot = RootHUD->GetRootCanvas()->AddChildToCanvas(BorderInstance);
    if (!Slot)
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowUI: Slot is nullptr!"));
        return nullptr;
    }
    ApplyLayoutPreset(Slot, BorderWidgetInfo.Layout);
    Slot->SetPosition(FVector2D(BorderWidgetInfo.Layout.Position.X + InventoryWidgets.Num() * 10, BorderWidgetInfo.Layout.Position.Y + InventoryWidgets.Num() * 10));

    FGuid InventoryID = InventoryData->GetID();
    InventoryWidgets.Add(InventoryData, BorderInstance);

    return BorderInstance;
}

void UUISubsystem::HideInventoryUI(UInventoryData* InventoryData)
{
    if (!IsValid(InventoryData))
    {
        return;
    }

    if (TObjectPtr<UUW_InventoryBorder>* Found = InventoryWidgets.Find(InventoryData))
    {
        if (IsValid(*Found))
        {
            (*Found)->RemoveFromParent();
        }
        InventoryWidgets.Remove(InventoryData);
    }
}

void UUISubsystem::ResetAllUI()
{
    for (auto& Pair : SingleWidgets)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->RemoveFromParent();
        }
    }
    SingleWidgets.Empty();

    for (auto& Pair : InventoryWidgets)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->RemoveFromParent();
        }
    }
    InventoryWidgets.Empty();

    if (IsValid(RootHUD))
    {
        RootHUD->RemoveFromParent();
        RootHUD = nullptr;
    }
}

UUW_ItemCommandMenu* UUISubsystem::ShowItemCommandMenu(UItemInstance* Item, const FVector2D& ScreenPos)
{
    InitRootHUD();

    FUIInfo WidgetInfo;
    if (!UIConfigData->GetInfo(EUIType::ItemCommandMenu, WidgetInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowItemCommandMenu: Cannot Find UIConfig Info!"));
        return nullptr;
    }

    if (!RootHUD || !RootHUD->GetRootCanvas() || !IsValid(Item))
    {
        return nullptr;
    }

    CloseItemCommandMenu();

    APlayerController* PC = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(GetWorld()) : nullptr;
    if (!PC)
    {
        return nullptr;
    }

    ActiveItemCommandMenu = CreateWidget<UUW_ItemCommandMenu>(PC, WidgetInfo.WidgetClass);
    if (!IsValid(ActiveItemCommandMenu))
    {
        return nullptr;
    }

    ActiveItemCommandMenu->InitMenu(Item);

    UCanvasPanelSlot* Slot = RootHUD->GetRootCanvas()->AddChildToCanvas(ActiveItemCommandMenu);
    ApplyLayoutPreset(Slot, WidgetInfo.Layout);

    Slot->SetPosition(ScreenPos);

    UE_LOG(LogTemp, Warning, TEXT("ShowItemCommandMenu called. Item=%s"), *GetNameSafe(Item));
    UE_LOG(LogTemp, Warning, TEXT("MenuClass=%s"), *GetNameSafe(WidgetInfo.WidgetClass));

    return ActiveItemCommandMenu;
}

UTCCarActivate* UUISubsystem::ShowCarCommandMenu(ATCCarBase* InCar, const FVector2D& ScreenPos)
{
    InitRootHUD();

    FUIInfo WidgetInfo;
    if (!UIConfigData->GetInfo(EUIType::CarInteraction, WidgetInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::ShowItemCommandMenu: Cannot Find UIConfig Info!"));
        return nullptr;
    }

    if (!RootHUD || !RootHUD->GetRootCanvas() || !IsValid(InCar))
    {
        return nullptr;
    }

    CloseCarInteractionMenu();

    APlayerController* PC = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(GetWorld()) : nullptr;
    if (!PC)
    {
        return nullptr;
    }

    CarActiveCommand = CreateWidget<UTCCarActivate>(PC, WidgetInfo.WidgetClass);
    if (!IsValid(CarActiveCommand))
    {
        return nullptr;
    }

    CarActiveCommand->Setup(InCar);

    UCanvasPanelSlot* Slot = RootHUD->GetRootCanvas()->AddChildToCanvas(CarActiveCommand);
    ApplyLayoutPreset(Slot, WidgetInfo.Layout);

    Slot->SetPosition(ScreenPos);

    return CarActiveCommand;
}

void UUISubsystem::CloseItemCommandMenu()
{
    if (IsValid(ActiveItemCommandMenu))
    {
        ActiveItemCommandMenu->RemoveFromParent();
    }
    ActiveItemCommandMenu = nullptr;
}

void UUISubsystem::CloseCarInteractionMenu()
{
    if (IsValid(CarActiveCommand))
    {
        CarActiveCommand->RemoveFromParent();
    }
    CarActiveCommand = nullptr;
}

void UUISubsystem::BindVoiceIndicator()
{
    if (!UIConfigData)
    {
        return;
    }

    if (!VoiceSubsystem)
    {
        if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
        {
            if (UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
            {
                VoiceSubsystem = GameInstance->GetSubsystem<UEOSVoiceChatSubsystem>();
            }
        }
    }

    if (!VoiceSubsystem)
    {
        return;
    }

    VoiceSubsystem->OnVoiceTransmitStateChanged.RemoveDynamic(this, &ThisClass::HandleVoiceTransmitStateChanged);
    VoiceSubsystem->OnVoiceTransmitStateChanged.AddDynamic(this, &ThisClass::HandleVoiceTransmitStateChanged);
    UpdateVoiceIndicatorUI(VoiceSubsystem->IsVoiceIndicatorActive());
}

void UUISubsystem::UpdateVoiceIndicatorUI(bool bIsActive)
{
    FUIInfo VoiceInfo;
    if (!UIConfigData || !UIConfigData->GetInfo(EUIType::VoiceIndicator, VoiceInfo))
    {
        return;
    }

    if (bIsActive)
    {
        ShowUI(EUIType::VoiceIndicator);
    }
    else
    {
        HideUI(EUIType::VoiceIndicator);
    }
}

void UUISubsystem::HandleVoiceTransmitStateChanged(bool bIsActive)
{
    UpdateVoiceIndicatorUI(bIsActive);
}

void UUISubsystem::InitRootHUD()
{
    APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::InitRootHUD: PC is null!"));
        return;
    }

    if (IsValid(RootHUD))
    {
        if (!RootHUD->IsInViewport())
        {
            RootHUD->AddToViewport();
            UE_LOG(LogTemp, Warning, TEXT("InitRootHUD: Re-AddToViewport RootHUD"));
        }
        return;
    }

    FUIInfo RootInfo;
    if (!UIConfigData->GetInfo(EUIType::Root, RootInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::InitRootHUD: Cannot Find UIConfig Info!"));
        return;
    }

    RootHUD = CreateWidget<UUW_RootHUD>(PC, RootInfo.WidgetClass);
    if (RootHUD == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("UUISubsystem::InitRootHUD: RootHUD is null!"));
        return;
    }
    RootHUD->OnGlobalMouseDown.AddUObject(this, &ThisClass::CloseItemCommandMenu);
    RootHUD->OnGlobalMouseDown.AddUObject(this, &ThisClass::CloseCarInteractionMenu);
    RootHUD->AddToViewport();
}

void UUISubsystem::ApplyLayoutPreset(UCanvasPanelSlot* Slot, const FUILayoutPreset& Layout)
{
    Slot->SetAutoSize(Layout.bAutoSize);
    Slot->SetAnchors(Layout.Anchors);
    Slot->SetAlignment(Layout.Alignment);
    Slot->SetPosition(Layout.Position);

    if (!Layout.bAutoSize)
    {
        Slot->SetSize(Layout.Size);
    }

    Slot->SetZOrder(Layout.ZOrder);
}
