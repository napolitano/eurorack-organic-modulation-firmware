#include "fmd/domain/DriftEngine.h"
namespace fmd {
Algorithm algorithmFromConfig(bool c1,bool c2) {
  if (!c1 && !c2) { return Algorithm::Perlin; }
  if (!c1 && c2) { return Algorithm::Brownian; }
  if (c1 && !c2) { return Algorithm::Bezier; }
  return Algorithm::Lfo;
}
DriftEngine::DriftEngine(Algorithm a,uint16_t seed,const IReferenceTables& t)
 : algorithm_(a),perlin_(t,seed),brownian_(seed),bezier_(t,seed),lfo_(t) {}
uint16_t DriftEngine::step(const ControlFrame& c) {
  switch(algorithm_) { case Algorithm::Perlin:return perlin_.step(c); case Algorithm::Brownian:return brownian_.step(c); case Algorithm::Bezier:return bezier_.step(c); case Algorithm::Lfo:return lfo_.step(c); }
  return 0;
}
}  // namespace fmd
