#pragma once

#include "scheme_type_id.h"
#include "scheme_type_info.h"

namespace NKikimr {
namespace NScheme {

enum class EOrder : ui16 {
    Ascending = 0,
    Descending = 1,
};

class TTypeIdOrder {
public:
    TTypeIdOrder() noexcept {
        TypeId_ = 0;
        Descending_ = 0;
    }

    /**
     * This allows implicit conversions from TTypeId
     */
    TTypeIdOrder(TTypeId typeId, EOrder order = EOrder::Ascending) noexcept {
        Set(typeId, order);
    }

    /**
     * This allows implicit conversions from TTypeId
     */
    TTypeIdOrder& operator=(TTypeId typeId) noexcept {
        return Set(typeId);
    }

    TTypeId GetTypeId() const noexcept { return TypeId_; }
    EOrder GetOrder() const noexcept { return EOrder(Descending_); }
    bool IsAscending() const noexcept { return !Descending_; }
    bool IsDescending() const noexcept { return Descending_; }

    TTypeIdOrder& Set(TTypeId typeId, EOrder order = EOrder::Ascending) noexcept {
        Y_VERIFY(typeId <= 0x7FFF, "Type id is out of bounds");

        TypeId_ = typeId;
        Descending_ = ui16(order);

        return *this;
    }

private:
    ui16 TypeId_ : 15;
    ui16 Descending_ : 1;
};

struct TTypeInfoOrder {
    TTypeInfoOrder()
    {}

    TTypeInfoOrder(TTypeIdOrder typeIdOrder, void* typeDesc = {})
        : TypeIdOrder(typeIdOrder)
        , TypeDesc(typeDesc)
    {}

    TTypeInfoOrder(TTypeInfo typeInfo, EOrder order = EOrder::Ascending)
        : TypeIdOrder(typeInfo.GetTypeId(), order)
        , TypeDesc(typeInfo.GetTypeDesc())
    {}

    TTypeId GetTypeId() const {
        return TypeIdOrder.GetTypeId();
    }

    EOrder GetOrder() const {
        return TypeIdOrder.GetOrder();
    }

    bool IsAscending() const {
        return TypeIdOrder.IsAscending();
    }

    bool IsDescending() const {
        return TypeIdOrder.IsDescending();
    }

    void* GetTypeDesc() const {
        return TypeDesc;
    }

    TTypeInfo ToTypeInfo() const {
        return TTypeInfo(GetTypeId(), GetTypeDesc());
    }

private:
    TTypeIdOrder TypeIdOrder;
    void* TypeDesc = {};
};

}
}
