#pragma once

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Food					UMETA(DisplayName = "Food"),				// 음식
	Clothing				UMETA(DisplayName = "Clothing"),			// 의류
	MeleeWeapon				UMETA(DisplayName = "MeleeWeapon"),			// 근접 무기
	FireArms				UMETA(DisplayName = "FireArms"),			// 총기류
	WeaponPart				UMETA(DisplayName = "WeaponPart"),			// 무기 부착물
	Ammo					UMETA(DisplayName = "Ammo"),				// 탄약
	Container				UMETA(DisplayName = "Container"),			// 가방, 케이스 등의 컨테이너
	Medical					UMETA(DisplayName = "Medical"),				// 의학용품
	Tool					UMETA(DisplayName = "Tool"),				// 도구 (제작, 건설 등)
	Cooking					UMETA(DisplayName = "Cooking"),				// 요리용품 등
	Communication			UMETA(DisplayName = "Communication"),		// 무전기 등
	Literature				UMETA(DisplayName = "Literature"),			// 책
	AnimalPart				UMETA(DisplayName = "AnimalPart"),			// 육류
	Material				UMETA(DisplayName = "Material"),			// 재료
	Appearance				UMETA(DisplayName = "Appearance"),			// 외형 (염색약, 마스카라 등 치장 잡템류)
	Junk					UMETA(DisplayName = "Junk"),				// 잡템
	VehicleMaintenance		UMETA(DisplayName = "Vehicle")	// 차량용 아이템
};

// 손으로 들 수 있는 아이템의 기능 카테고리
UENUM(BlueprintType, meta = (Bitflags))
enum class EItemCategory : uint8
{
	None			= 0				UMETA(DisplayName = "None"),					// 들 수는 있지만 기능 X
	Tool			= 1	<< 0		UMETA(DisplayName = "Tool"),					// 도구 상호작용
	Weapon			= 1 << 1		UMETA(DisplayName = "Weapon"),					// 공격 가능
};
ENUM_CLASS_FLAGS(EItemCategory);

// 어떤 속성을 가지고 있는지 (속성에 따라 테이블 참조 및 아이템 인스턴스에 컴포넌트 추가)
// 중복 가능하게 해야 하면 bitflag 사용하거나 FGameplayTag로 변경
UENUM(BlueprintType)
enum class EItemComponent : uint8
{
	None							UMETA(DisplayName = "None"),

	Food							UMETA(DisplayName = "Food"),				// 섭취류
	MeleeWeapon						UMETA(DisplayName = "MeleeWeapon"),			// 근접 무기
	Firearms						UMETA(DisplayName = "Firearms"),			// 총기류
	Tool							UMETA(DisplayName = "Tool"),				// 도구류
};

UENUM(BlueprintType)
enum class EHoldableSocket : uint8
{
	RightHand,
	LeftHand,
};

UENUM(BlueprintType)
enum class EHoldableType : uint8
{
	None,					// default AnimPreset으로 설정하기 위해 사용
	MeleeWeaponR,
	Pistol,
};

UENUM()
enum class EBodyLocation : uint32
{
	None				= 0				UMETA(DisplayName = "None"),

#pragma region BaseSlot

	// 머리
	Head				= 1	<< 0		UMETA(DisplayName = "Head"),				// 머리 장비 (헬멧, 모자 등)
	Face				= 1 << 1		UMETA(DisplayName = "Face"),				// 마스크, 고글 등
	Eyes				= 1 << 2		UMETA(DisplayName = "Eyes"),				// 안경류
	Ear					= 1 << 3		UMETA(DisplayName = "Ear"),					// 헤드셋, 귀걸이 등
	Neck				= 1 << 4		UMETA(DisplayName = "Neck"),				// 목도리, 목 방어구 등
	Nose				= 1 << 5		UMETA(DisplayName = "Nose"),				// 코 피어싱 등

	// 가방류
	Back				= 1 << 6		UMETA(DisplayName = "Back"),				// 배낭류

	// 의복 - 상의 (Tops)
	// (속옷) -> TShirts -> ShortSleeveShirt -> Shirt -> Sweater -> TorsoExtra
	TShirts				= 1 << 7		UMETA(DisplayName = "TShirts"),				// 티셔츠 (속옷 위, 짧은 셔츠 아래)
	ShortSleeveShirt	= 1 << 8		UMETA(DisplayName = "ShortSleeveShirt"),	// 짧은 셔츠 (티셔츠 위, 셔츠 아래)
	Shirt				= 1 << 9		UMETA(DisplayName = "Shirt"),				// 셔츠 (짧은 셔츠 위, 외투 아래)
	Sweater				= 1 << 10		UMETA(DisplayName = "Sweater"),				// 스웨터, 후드집업 (셔츠 위, 외투 아래)
	TorsoExtra			= 1 << 11		UMETA(DisplayName = "TorsoExtra"),			// 조끼, 앞치마 등 (스웨터 위)

	// 의복 - 하의
	Bottoms				= 1 << 12		UMETA(DisplayName = "Bottoms"),				// 하의 (속옷 위)

	// 의복 - 발
	Socks				= 1 << 13		UMETA(DisplayName = "Socks"),				// 양말
	Shoes				= 1 << 14		UMETA(DisplayName = "Shoes"),				// 신발

	// 의복 - 속옷
	UnderWearTop		= 1 << 15		UMETA(DisplayName = "UnderWearTop"),		// 속옷 상의
	UnderWearBottom		= 1 << 16		UMETA(DisplayName = "UnderWearBottom"),		// 속옷 하의

	// 의복 - 내복
	InnerWearTop		= 1 << 17		UMETA(DisplayName = "InnerWearTop"),		// 내복 상의
	InnerWearBottom		= 1 << 18		UMETA(DisplayName = "InnerWearBottom"),		// 내복 하의

	// 의복 - 손
	Gloves				= 1 << 19		UMETA(DisplayName = "Gloves"),				// 장갑류
	RightWrist			= 1 << 20		UMETA(DisplayName = "RightWrist"),			// 오른 손목
	LeftWrist			= 1 << 21		UMETA(DisplayName = "LeftWrist"),			// 왼 손목

	RightHand			= 1 << 22		UMETA(DisplayName = "RightHand"),			// 오른손
	LeftHand			= 1 << 23		UMETA(DisplayName = "LeftHand"),			// 왼손

	MAX_BASE			= 1 << 24		UMETA(Hidden),								// 종료

#pragma endregion

#pragma region ComplexSlot

	Hands				= RightHand | LeftHand		UMETA(DisplayName = "Hands"),	// 양 손

#pragma endregion
};

FORCEINLINE bool Exclusive(EBodyLocation Slot1, EBodyLocation Slot2)
{
	return (static_cast<uint32>(Slot1) & static_cast<uint32>(Slot2)) != 0;
}