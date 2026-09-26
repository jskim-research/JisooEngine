#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

struct FVector
{
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;

    constexpr FVector() = default;
    constexpr FVector(float InX, float InY, float InZ)
        : X(InX), Y(InY), Z(InZ)
    {
    }

    [[nodiscard]] constexpr FVector operator+(const FVector& Other) const
    {
        return {X + Other.X, Y + Other.Y, Z + Other.Z};
    }

    [[nodiscard]] constexpr FVector operator-(const FVector& Other) const
    {
        return {X - Other.X, Y - Other.Y, Z - Other.Z};
    }

    [[nodiscard]] constexpr FVector operator*(float Scalar) const
    {
        return {X * Scalar, Y * Scalar, Z * Scalar};
    }
};

struct FQuat
{
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float W = 1.0f;

    constexpr FQuat() = default;
    constexpr FQuat(float InX, float InY, float InZ, float InW)
        : X(InX), Y(InY), Z(InZ), W(InW)
    {
    }
};

struct FMatrix
{
    float M[4][4]{};

    [[nodiscard]] static constexpr FMatrix Identity()
    {
        FMatrix Result{};
        Result.M[0][0] = 1.0f;
        Result.M[1][1] = 1.0f;
        Result.M[2][2] = 1.0f;
        Result.M[3][3] = 1.0f;
        return Result;
    }

    [[nodiscard]] FMatrix operator*(const FMatrix& Other) const
    {
        FMatrix Result{};
        for (int Row = 0; Row < 4; ++Row)
        {
            for (int Column = 0; Column < 4; ++Column)
            {
                for (int Index = 0; Index < 4; ++Index)
                {
                    Result.M[Row][Column] += M[Row][Index] * Other.M[Index][Column];
                }
            }
        }
        return Result;
    }

    [[nodiscard]] FVector TransformPosition(const FVector& Position) const
    {
        return {
            Position.X * M[0][0] + Position.Y * M[1][0] + Position.Z * M[2][0] + M[3][0],
            Position.X * M[0][1] + Position.Y * M[1][1] + Position.Z * M[2][1] + M[3][1],
            Position.X * M[0][2] + Position.Y * M[1][2] + Position.Z * M[2][2] + M[3][2]};
    }
};

struct FTransform
{
    FVector Translation{};
    FQuat Rotation{};
    FVector Scale3D{1.0f, 1.0f, 1.0f};

    [[nodiscard]] FMatrix ToMatrixWithScale() const
    {
        const float LengthSquared = Rotation.X * Rotation.X
            + Rotation.Y * Rotation.Y
            + Rotation.Z * Rotation.Z
            + Rotation.W * Rotation.W;
        const float InverseLength = LengthSquared > 0.0f
            ? 1.0f / std::sqrt(LengthSquared)
            : 1.0f;

        const float X = Rotation.X * InverseLength;
        const float Y = Rotation.Y * InverseLength;
        const float Z = Rotation.Z * InverseLength;
        const float W = Rotation.W * InverseLength;

        FMatrix Result = FMatrix::Identity();
        Result.M[0][0] = Scale3D.X * (1.0f - 2.0f * (Y * Y + Z * Z));
        Result.M[0][1] = Scale3D.X * (2.0f * (X * Y + Z * W));
        Result.M[0][2] = Scale3D.X * (2.0f * (X * Z - Y * W));
        Result.M[1][0] = Scale3D.Y * (2.0f * (X * Y - Z * W));
        Result.M[1][1] = Scale3D.Y * (1.0f - 2.0f * (X * X + Z * Z));
        Result.M[1][2] = Scale3D.Y * (2.0f * (Y * Z + X * W));
        Result.M[2][0] = Scale3D.Z * (2.0f * (X * Z + Y * W));
        Result.M[2][1] = Scale3D.Z * (2.0f * (Y * Z - X * W));
        Result.M[2][2] = Scale3D.Z * (1.0f - 2.0f * (X * X + Y * Y));
        Result.M[3][0] = Translation.X;
        Result.M[3][1] = Translation.Y;
        Result.M[3][2] = Translation.Z;
        return Result;
    }
};

struct FBox
{
    FVector Min{};
    FVector Max{};

    FBox() = default;
    FBox(const FVector& InMin, const FVector& InMax)
        : Min(InMin), Max(InMax)
    {
    }

    [[nodiscard]] FBox TransformBy(const FMatrix& Matrix) const
    {
        FVector NewMin{
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()};
        FVector NewMax{
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest()};

        for (int CornerIndex = 0; CornerIndex < 8; ++CornerIndex)
        {
            const FVector Corner{
                (CornerIndex & 1) != 0 ? Max.X : Min.X,
                (CornerIndex & 2) != 0 ? Max.Y : Min.Y,
                (CornerIndex & 4) != 0 ? Max.Z : Min.Z};
            const FVector Transformed = Matrix.TransformPosition(Corner);
            NewMin.X = (std::min)(NewMin.X, Transformed.X);
            NewMin.Y = (std::min)(NewMin.Y, Transformed.Y);
            NewMin.Z = (std::min)(NewMin.Z, Transformed.Z);
            NewMax.X = (std::max)(NewMax.X, Transformed.X);
            NewMax.Y = (std::max)(NewMax.Y, Transformed.Y);
            NewMax.Z = (std::max)(NewMax.Z, Transformed.Z);
        }

        return {NewMin, NewMax};
    }
};
