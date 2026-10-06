#include "Runtime/Render/Mesh/MeshBatchCollection.h"

#include "Runtime/Render/Mesh/MeshBatch.h"
#include "Runtime/Render/Scene/PrimitiveSceneProxy.h"
#include "Runtime/Render/Scene/Scene.h"

void GatherVisibleMeshBatches(
    const FScene& Scene,
    FMeshBatchCollector& Collector)
{
    Scene.ForEachPrimitive(
        [&Collector](const FPrimitiveSceneProxy& Proxy)
        {
            if (Proxy.IsVisible())
            {
                Proxy.GatherMeshBatches(Collector);
            }
        });
}
