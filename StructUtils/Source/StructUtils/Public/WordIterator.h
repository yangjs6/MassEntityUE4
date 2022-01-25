// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/BitArray.h"

template<typename WordType>
struct TWordIteratorBase
{
	explicit operator bool() const
	{
		return CurrentIndex < NumWords;
	}

	int32 GetIndex() const
	{
		return CurrentIndex;
	}

	uint32 GetWord() const
	{
		checkSlow(CurrentIndex < NumWords);

		if (CurrentMask == ~0u)
		{
			return Data[CurrentIndex];
		}
		else if (MissingBitsFill == 0)
		{
			return Data[CurrentIndex] & CurrentMask;
		}
		else
		{
			return (Data[CurrentIndex] & CurrentMask) | (MissingBitsFill & ~CurrentMask);
		}
	}

	void operator++()
	{
		++this->CurrentIndex;
		if (this->CurrentIndex == NumWords - 1)
		{
			CurrentMask = FinalMask;
		}
		else
		{
			CurrentMask = ~0u;
		}
	}

	void FillMissingBits(uint32 InMissingBitsFill)
	{
		MissingBitsFill = InMissingBitsFill;
	}

protected:

	explicit TWordIteratorBase(WordType* InData, int32 InStartBitIndex, int32 InEndBitIndex)
		: Data(InData)
		, CurrentIndex(InStartBitIndex / NumBitsPerDWORD)
		, NumWords(FMath::DivideAndRoundUp(InEndBitIndex, NumBitsPerDWORD))
		, CurrentMask(~0u << (InStartBitIndex % NumBitsPerDWORD))
		, FinalMask(~0u)
		, MissingBitsFill(0)
	{
		const int32 Shift = NumBitsPerDWORD - (InEndBitIndex % NumBitsPerDWORD);
		if (Shift < NumBitsPerDWORD)
		{
			FinalMask = ~0u >> Shift;
		}

		if (CurrentIndex == NumWords - 1)
		{
			CurrentMask &= FinalMask;
			FinalMask = CurrentMask;
		}
	}

	WordType* RESTRICT Data;

	int32 CurrentIndex;
	int32 NumWords;

	uint32 CurrentMask;
	uint32 FinalMask;
	uint32 MissingBitsFill;
};

template<typename Allocator = FDefaultBitArrayAllocator>
struct FConstWordIterator : TWordIteratorBase<const uint32>
{
	explicit FConstWordIterator(const TBitArray<Allocator>& InArray)
		: TWordIteratorBase<const uint32>(InArray.GetData(), 0, InArray.Num())
	{}

	explicit FConstWordIterator(const TBitArray<Allocator>& InArray, int32 InStartBitIndex, int32 InEndBitIndex)
		: TWordIteratorBase<const uint32>(InArray.GetData(), InStartBitIndex, InEndBitIndex)
	{
		checkSlow(InStartBitIndex <= InEndBitIndex && InStartBitIndex <= InArray.Num() && InEndBitIndex <= InArray.Num());
		checkSlow(InStartBitIndex >= 0 && InEndBitIndex >= 0);
	}
};

template<typename Allocator = FDefaultBitArrayAllocator>
struct FWordIterator : TWordIteratorBase<uint32>
{
	explicit FWordIterator(TBitArray<Allocator>& InArray)
		: TWordIteratorBase<uint32>(InArray.GetData(), 0, InArray.Num())
	{}

	void SetWord(uint32 InWord)
	{
		checkSlow(this->CurrentIndex < this->NumWords);

		if (this->CurrentIndex == this->NumWords - 1)
		{
			this->Data[this->CurrentIndex] = InWord & this->FinalMask;
		}
		else
		{
			this->Data[this->CurrentIndex] = InWord;
		}
	}
};