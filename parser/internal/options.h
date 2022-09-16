#ifndef COBOLD_PARSER_INTERNAL_OPTIONS
#define COBOLD_PARSER_INTERNAL_OPTIONS

namespace cobold {

inline constexpr int kDefaultErrorRecoveryLimit = 30;
inline constexpr int kDefaultMaxRecursionDepth = 250;
inline constexpr int kExpressionSizeCodepointLimit = 100'000;
inline constexpr int kDefaultErrorRecoveryTokenLookaheadLimit = 512;
inline constexpr bool kDefaultAddMacroCalls = false;

}  // namespace cobold

#endif /* COBOLD_PARSER_INTERNAL_OPTIONS */
