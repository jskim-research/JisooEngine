#pragma once

#include "Runtime/CoreUObject/Object.h"

#include "CoreUObject/ExampleObject.generated.h"

UCLASS()
class UExampleObject final : public UObject
{
    GENERATED_BODY()

public:
    ~UExampleObject() override;

    void BeginDestroy() override;
    void FinishDestroy() override;

    static void ResetCounters();
    [[nodiscard]] static int GetBeginDestroyCount();
    [[nodiscard]] static int GetFinishDestroyCount();
    [[nodiscard]] static int GetDestructorCount();

protected:
    explicit UExampleObject(const FObjectInitializer& ObjectInitializer);

private:
    static int BeginDestroyCount;
    static int FinishDestroyCount;
    static int DestructorCount;
};
