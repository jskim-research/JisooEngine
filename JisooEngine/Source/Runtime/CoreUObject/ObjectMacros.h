#pragma once

#define UCLASS(...)

#define UE_CONCAT_INNER(A, B) A##B
#define UE_CONCAT(A, B) UE_CONCAT_INNER(A, B)
#define UE_CONCAT_FOUR_INNER(A, B, C, D) A##B##C##D
#define UE_CONCAT_FOUR(A, B, C, D) UE_CONCAT_FOUR_INNER(A, B, C, D)
#define UE_EXPAND(Value) Value

#ifndef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_MissingGeneratedHeader
#endif

#define GENERATED_BODY() \
    UE_EXPAND(UE_CONCAT_FOUR(CURRENT_FILE_ID, _, __LINE__, _GENERATED_BODY))
