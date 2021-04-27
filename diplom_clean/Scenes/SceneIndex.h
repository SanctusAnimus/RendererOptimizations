#pragma once
#ifndef SCENES_INDEX_DECL
#define SCENES_INDEX_DECL

#include "RawScene.h"			// 1. no optimizations [disable instancing]
#include "InstancingScene.h"	// 2. instancing [disable deferred]
#include "DeferredScene.h"		// 3. deferred shading [disable frustum]
#include "FrustumScene.h"		// 4. frustum culling  [disable texture comp]
#include "StartingScene.h"		// 5. all optimizations (adding last - texture compression)
#include "LoadableScene.h"		// 6. scene which content can be (de)serialized

#endif // !SCENES_INDEX_DECL
