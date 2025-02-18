#include "registry.h"

#include <ydb/library/yql/core/arrow_kernels/registry/registry.h>
#include <ydb/library/yql/minikql/invoke_builtins/mkql_builtins.h>
#include <ydb/library/yql/minikql/comp_nodes/mkql_factories.h>

namespace NKikimr::NOlap {

bool TKernelsRegistry::Parse(const TString& serialized) {
    auto functionRegistry = NMiniKQL::CreateFunctionRegistry(NMiniKQL::CreateBuiltinRegistry());
    auto nodeFactory = NMiniKQL::GetBuiltinFactory();
    auto kernels =  NYql::LoadKernels(serialized, *functionRegistry, nodeFactory);
    Kernels.swap(kernels);
    for (const auto& kernel : Kernels) {
        arrow::compute::Arity arity(kernel->signature->in_types().size(), kernel->signature->is_varargs());
        auto func = std::make_shared<arrow::compute::ScalarFunction>("local_function", arity, nullptr);
        auto res = func->AddKernel(*kernel);
        if (!res.ok()) {
            return false;
        }
        Functions.push_back(func);
    }
    return true;
}   
}
