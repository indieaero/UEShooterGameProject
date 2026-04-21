// Shoot Them Up Game, All Rights Reserved.

#include "STULocalPlayerPendingDisplayNameSubsystem.h"

namespace STULocalPlayerName
{
    static constexpr int32 MaxDisplayNameLen = 48;

    static void Sanitize(FString& InOut)
    {
        InOut.TrimStartAndEndInline();
        InOut.ReplaceInline(TEXT("\r"), TEXT(""), ESearchCase::CaseSensitive);
        InOut.ReplaceInline(TEXT("\n"), TEXT(" "), ESearchCase::CaseSensitive);
        if (InOut.Len() > MaxDisplayNameLen)
        {
            InOut.LeftInline(MaxDisplayNameLen);
        }
    }
}  // namespace STULocalPlayerName

void USTULocalPlayerPendingDisplayNameSubsystem::SetPendingDisplayName(const FString& Name)
{
    FString Copy = Name;
    STULocalPlayerName::Sanitize(Copy);
    PendingDisplayName = MoveTemp(Copy);
}
