#pragma once

class FMeshBatchCollector;
class FScene;

/**
 * Scene에 등록된 Visible Proxy만 순회해 현재 프레임의 MeshBatch를 수집한다.
 * Frustum이 없는 현재 단계에서는 Proxy의 Visibility flag만 판정한다.
 */
void GatherVisibleMeshBatches(
    const FScene& Scene,
    FMeshBatchCollector& Collector);
