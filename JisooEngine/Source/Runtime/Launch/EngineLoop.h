#pragma once

class FEngine;

/** Window와 Engine의 초기화, 반복 Tick, 종료 순서를 관리한다. */
class FEngineLoop
{
public:
    /**
     * Window가 닫힐 때까지 현재 Thread에서 Engine을 실행한다.
     *
     * @return 정상 종료하면 0, Window 또는 Engine 초기화에 실패하면 -1
     */
    int Run(FEngine& Engine);
};
