#pragma once
#include "irgen_context.hpp"

namespace flux {

class IRGenContext;
class TypeConverter;

RuntimeDecls DeclareRuntimeDecls(IRGenContext &ctx, TypeConverter &types);

} // namespace flux
