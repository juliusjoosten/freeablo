#include "nuklear_sdl_gl3.h"
#include "../../apps/freeablo/fagui/guimanager.h"
#include <cstring>
#include <misc/assert.h>
#include <misc/misc.h>
#include <render/OpenGL/pipelineopengl.h>
#include <render/OpenGL/textureopengl.h>
#include <render/commandqueue.h>
#include <render/renderinstance.h>
#include <render/texture.h>
#include <render/vertexarrayobject.h>
#include <render/vertextypes.h>

static const struct nk_draw_vertex_layout_element vertex_layout[] = {{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct Render::NuklearVertex, position)},
                                                                     {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct Render::NuklearVertex, uv)},
                                                                     {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct Render::NuklearVertex, color)},
                                                                     {NK_VERTEX_LAYOUT_END}};

NuklearDevice::NuklearDevice(Render::RenderInstance& renderInstance)
{
    nk_buffer_init_default(&commands);

    Render::PipelineSpec pipelineSpec;
    pipelineSpec.vertexLayouts = {Render::NuklearVertex::layout()};
    pipelineSpec.vertexShaderPath = Misc::getResourcesPath().str() + "/shaders/gui.vert";
    pipelineSpec.fragmentShaderPath = Misc::getResourcesPath().str() + "/shaders/gui.frag";
    pipelineSpec.descriptorSetSpec = {{
        {Render::DescriptorType::UniformBuffer, "vertexUniforms"},
        {Render::DescriptorType::UniformBuffer, "fragmentUniforms"},
        {Render::DescriptorType::Texture, "Texture"},
    }};
    pipelineSpec.scissor = true;

    pipeline = renderInstance.createPipeline(pipelineSpec);
    vertexArrayObject = renderInstance.createVertexArrayObject({0}, pipelineSpec.vertexLayouts, 1);
    descriptorSet = renderInstance.createDescriptorSet(pipelineSpec.descriptorSetSpec);
    uniformCpuBuffer = std::make_unique<GuiUniforms::CpuBufferType>(renderInstance.capabilities().uniformBufferOffsetAlignment);
    uniformBuffer = renderInstance.createBuffer(uniformCpuBuffer->getSizeInBytes());
}

NuklearDevice::~NuklearDevice() { nk_buffer_free(&commands); }

void NuklearFrameDump::render(Vec2i screenResolution, Render::CommandQueue& commandQueue)
{
    float ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, -2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= float(screenResolution.w);
    ortho[1][1] /= float(screenResolution.h);

    // convert from command queue into draw list and draw to screen
    size_t offset = 0;

    mDevice.vertexArrayObject->getVertexBuffer(0)->setData(mVertexBuffer.memory.ptr, mVertexBuffer.size);
    mDevice.vertexArrayObject->getIndexBuffer()->setData(mIndexBuffer.memory.ptr, mIndexBuffer.size);

    // clang-format off
    mDevice.descriptorSet->updateItems({
        {0, Render::BufferSlice{mDevice.uniformBuffer.get(), mDevice.uniformCpuBuffer->getMemberOffset<GuiUniforms::Vertex>(), sizeof(GuiUniforms::Vertex)}},
        {1, Render::BufferSlice{mDevice.uniformBuffer.get(), mDevice.uniformCpuBuffer->getMemberOffset<GuiUniforms::Fragment>(), sizeof(GuiUniforms::Fragment)}},
    });
    // clang-format on

    // iterate over and execute each draw command
    for (const auto& cmd : mDrawCommands)
    {
        if (!cmd.elem_count)
            continue;

        auto effect = static_cast<FAGui::EffectType>(cmd.userdata.id);

        int item_hl_color[] = {0xB9, 0xAA, 0x77};

        GuiUniforms::Vertex* vertex = mDevice.uniformCpuBuffer->getMemberPointer<GuiUniforms::Vertex>();
        memcpy(vertex->ProjMtx, ortho, sizeof(ortho));

        GuiUniforms::Fragment* fragment = mDevice.uniformCpuBuffer->getMemberPointer<GuiUniforms::Fragment>();
        fragment->hoverColor[0] = float(item_hl_color[0]) / 255.0f;
        fragment->hoverColor[1] = float(item_hl_color[1]) / 255.0f;
        fragment->hoverColor[2] = float(item_hl_color[2]) / 255.0f;
        fragment->hoverColor[3] = effect == FAGui::EffectType::highlighted ? 1.0f : 0.0f;
        fragment->checkerboarded = effect == FAGui::EffectType::checkerboarded ? 1.0f : 0.0f;

        auto* nuklearTextureHandle = reinterpret_cast<FANuklearTextureHandle*>(cmd.texture.ptr);
        const Render::AtlasTextureEntry* atlasEntry = nuklearTextureHandle->spriteGroup;

        if (atlasEntry)
        {
            mDevice.descriptorSet->updateItems({
                {2, atlasEntry->mTexture},
            });

            fragment->imageSize[0] = atlasEntry->mWidth;
            fragment->imageSize[1] = atlasEntry->mHeight;
            fragment->atlasSize[0] = atlasEntry->mTexture->width();
            fragment->atlasSize[1] = atlasEntry->mTexture->height();
            fragment->atlasOffset[0] = atlasEntry->mX;
            fragment->atlasOffset[1] = atlasEntry->mY;
            fragment->atlasOffset[2] = atlasEntry->mLayer;
        }
        else
        {
            mDevice.descriptorSet->updateItems({
                {2, nuklearTextureHandle->texture},
            });

            fragment->imageSize[0] = nuklearTextureHandle->texture->width();
            fragment->imageSize[1] = nuklearTextureHandle->texture->height();
            fragment->atlasSize[0] = nuklearTextureHandle->texture->width();
            fragment->atlasSize[1] = nuklearTextureHandle->texture->height();
            fragment->atlasOffset[0] = 0;
            fragment->atlasOffset[1] = 0;
            fragment->atlasOffset[2] = 0;
        }

        mDevice.uniformBuffer->setData(mDevice.uniformCpuBuffer->data(), mDevice.uniformCpuBuffer->getSizeInBytes());

        Render::ScissorRect scissor = {};
        scissor.x = int32_t(cmd.clip_rect.x);
        scissor.y = int32_t((float(screenResolution.h) - (cmd.clip_rect.y + cmd.clip_rect.h)));
        scissor.w = int32_t(cmd.clip_rect.w);
        scissor.h = int32_t(cmd.clip_rect.h);

        commandQueue.cmdScissor(scissor);

        Render::Bindings bindings;
        bindings.vao = mDevice.vertexArrayObject.get();
        bindings.pipeline = mDevice.pipeline.get();
        bindings.descriptorSet = mDevice.descriptorSet.get();

        commandQueue.cmdDrawIndexed(size_t(offset), cmd.elem_count, bindings);

        // Useful if we ever need to debug the generated vertices:

        //  std::vector<nk_draw_index> indices;
        //  indices.resize(cmd.elem_count);
        //  memcpy(indices.data(), ((char*)mIndexBuffer.memory.ptr) + offset, cmd.elem_count);
        //
        //  auto* allVertices = (Render::NuklearVertex*)mVertexBuffer.memory.ptr;
        //  std::vector<Render::NuklearVertex> vertices;
        //  vertices.resize(indices.size());
        //  for (int32_t i = 0; i < int32_t(vertices.size()); i++)
        //      vertices[i] = allVertices[indices[i]];

        offset += cmd.elem_count * sizeof(nk_draw_index);
    }
}

NuklearFrameDump::NuklearFrameDump(NuklearDevice& dev) : mDevice(dev)
{
    nk_buffer_init_default(&mVertexBuffer);
    nk_buffer_init_default(&mIndexBuffer);
    nk_buffer_init_default(&mCommandsTemp);

    memset(&mConvertConfig, 0, sizeof(mConvertConfig));
    mConvertConfig.vertex_layout = vertex_layout;
    mConvertConfig.vertex_size = sizeof(Render::NuklearVertex);
    mConvertConfig.vertex_alignment = NK_ALIGNOF(Render::NuklearVertex);
    mConvertConfig.null = dev.nullTexture;
    mConvertConfig.circle_segment_count = 22;
    mConvertConfig.curve_segment_count = 22;
    mConvertConfig.arc_segment_count = 22;
    mConvertConfig.global_alpha = 1.0f;
    mConvertConfig.shape_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON;
    mConvertConfig.line_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON;
}

NuklearFrameDump::~NuklearFrameDump()
{
    nk_buffer_free(&mVertexBuffer);
    nk_buffer_free(&mIndexBuffer);
    nk_buffer_free(&mCommandsTemp);
}

void NuklearFrameDump::fill(nk_context* ctx)
{
    nk_buffer_clear(&mVertexBuffer);
    nk_buffer_clear(&mIndexBuffer);
    nk_buffer_clear(&mCommandsTemp);
    nk_convert(ctx, &mCommandsTemp, &mVertexBuffer, &mIndexBuffer, &mConvertConfig);

    mDrawCommands.clear();

    const nk_draw_command* cmd;
    nk_draw_foreach(cmd, ctx, &mCommandsTemp) { mDrawCommands.push_back(*cmd); }
}
