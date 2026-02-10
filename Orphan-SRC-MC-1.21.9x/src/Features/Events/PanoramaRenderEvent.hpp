//
// Created by EDest10 6/15/2025
//

class PanoramaRenderer;              // forward declaration
class MinecraftUIRenderContext;     // forward declaration
class ClientInstance;               // forward declaration

class PanoramaRenderEvent : public CancelableEvent {
public:
    PanoramaRenderer* mThat{};
    MinecraftUIRenderContext* mRenderContext{};
    ClientInstance* mClient{};

    explicit PanoramaRenderEvent(PanoramaRenderer *that, MinecraftUIRenderContext *renderContext,
                                 ClientInstance *client, struct UIControl *control)
            : mThat(that), mRenderContext(renderContext), mClient(client) {}
};