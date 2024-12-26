#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "TransparentHeavyLevel.generated.h"

UCLASS()
class GRADWORK_API ATransparentHeavyLevel : public ALevelScriptActor
{
	GENERATED_BODY()

public:
	ATransparentHeavyLevel();
	
protected:
	virtual void BeginPlay() override;

private:
	UStaticMesh* m_pCubeMesh{ nullptr };
	UMaterialInterface* m_pBaseMaterial{ nullptr };
	
	void SpawnCubes();
};
