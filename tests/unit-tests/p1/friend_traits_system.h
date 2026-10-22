// Model a dependency whose declaration subtree the extractor deliberately skips.
#pragma clang system_header

namespace p1_system {
// Alias and direct-class controls must never share a target: a direct visit
// would otherwise populate traits and mask a broken alias-only extraction.
class AliasPodTarget {};
class AliasNonPodTarget {
public:
  virtual ~AliasNonPodTarget() = default;
};
class AliasForwardTarget;
using PodAlias = AliasPodTarget;
using NonPodAlias = AliasNonPodTarget;
using ForwardAlias = AliasForwardTarget;

class DirectPodTarget {};
class DirectNonPodTarget {
public:
  virtual ~DirectNonPodTarget() = default;
};
class DirectForwardTarget;
} // namespace p1_system
