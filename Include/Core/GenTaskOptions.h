// Include/Core/GenTaskOptions.h
// ============================================================================
// @brief 生成式任务参数结构体 — 对标 ATOptions (Include/Core/ATOptions.h)
//        GenTaskParams  — 生成参数 (prompt/texture_size/...), 自带 JSON 序列化
//        GenTaskOptions — 嵌入 BlockObject::Task_Info (block_task_category + params)
// ============================================================================
#pragma once

#include <string>
#include "Core/Rapidjson.h"
#include "Core/String.h"     // StringIsNullOrBlank

namespace AI3D
{
    namespace CORE
    {
        // ============================================================================
        // GenTaskStatus — 生成式任务生命周期状态
        // 放在 Core/ 层供 GenTaskAPI (MoldAIData.dll) 和 GenTaskThread (MoldAINode.exe) 共用
        // ============================================================================
        enum class GenTaskStatus
        {
            UNKNOWN = -1, // 未指定 / 非法值, 调用者需过滤
            IDLE, // 初始 / 网络不通
            PENDING, // 已写入 jobs_gen/Pending/, 等待 GenTaskThread pick up
            IN_PROGRESS, // 已 POST submit 到服务端, 正在周期性轮询
            COMPLETED, // 服务端返回完成
            FAILED, // 执行失败
            CANCELLED, // 用户取消
        };

        // ============================================================================
        // GenTaskSubType — 生成式任务类型 (扁平枚举, 对应 Triverse 10 个端点)
        // ToString() 返回 URL 路径段 (枚举名小写 + _→-), 由 GenHttpClient 拼完整 URL
        // SubTypeFromString() 接受 URL 格式, 反序列化回枚举
        // ============================================================================
        enum class GenTaskSubType
        {
            UNKNOWN = -1, // 未指定 / 非法值, 调用者需过滤
            TEXT_TO_MODEL, // → text-to-model     文字→带纹理模型
            TEXT_TO_MESH, // → text-to-mesh      文字→纯白模
            IMAGE_TO_MODEL, // → image-to-model    图片→带纹理模型
            IMAGE_TO_MESH, // → image-to-mesh     图片→纯白模
            TEXTURE_MODEL, // → texture-model     图片+模型→纹理
            TEXT_TO_TEXTURE, // → text-to-texture   文字+模型→纹理
            MODEL_PREVIEW_RENDER, // → model-preview-render  模型→预览图
            MODEL_REMESH, // → model-remesh          重网格/减面
            CONVERT_MODEL_FORMAT, // → convert-model-format  格式转换
            IMAGE_GENERATION, // → image-generation      文字→图片
        };

        // 枚举 → URL 路径段: 小写 + 下划线换横线
        inline const char* ToString(GenTaskSubType st)
        {
            switch (st)
            {
            case GenTaskSubType::TEXT_TO_MODEL: return "text-to-model";
            case GenTaskSubType::TEXT_TO_MESH: return "text-to-mesh";
            case GenTaskSubType::IMAGE_TO_MODEL: return "image-to-model";
            case GenTaskSubType::IMAGE_TO_MESH: return "image-to-mesh";
            case GenTaskSubType::TEXTURE_MODEL: return "texture-model";
            case GenTaskSubType::TEXT_TO_TEXTURE: return "text-to-texture";
            case GenTaskSubType::MODEL_PREVIEW_RENDER: return "model-preview-render";
            case GenTaskSubType::MODEL_REMESH: return "model-remesh";
            case GenTaskSubType::CONVERT_MODEL_FORMAT: return "convert-model-format";
            case GenTaskSubType::IMAGE_GENERATION: return "image-generation";
            default: return nullptr;
            }
        }

        inline GenTaskSubType SubTypeFromString(const std::string& s)
        {
            if (s == "text-to-model") return GenTaskSubType::TEXT_TO_MODEL;
            if (s == "text-to-mesh") return GenTaskSubType::TEXT_TO_MESH;
            if (s == "image-to-model") return GenTaskSubType::IMAGE_TO_MODEL;
            if (s == "image-to-mesh") return GenTaskSubType::IMAGE_TO_MESH;
            if (s == "texture-model") return GenTaskSubType::TEXTURE_MODEL;
            if (s == "text-to-texture") return GenTaskSubType::TEXT_TO_TEXTURE;
            if (s == "model-preview-render") return GenTaskSubType::MODEL_PREVIEW_RENDER;
            if (s == "model-remesh") return GenTaskSubType::MODEL_REMESH;
            if (s == "convert-model-format") return GenTaskSubType::CONVERT_MODEL_FORMAT;
            if (s == "image-generation") return GenTaskSubType::IMAGE_GENERATION;
            return GenTaskSubType::UNKNOWN;
        }

        // ============================================================================
        // GenTaskParams — 生成式任务的具体参数
        // 前端填充 → WriteToJson()/ToJsonString() → JSON → HTTP 请求 / BIN 持久化
        // ============================================================================
        struct GenTaskParams // 不加 AI3D_API — 纯数据 struct, 隐式拷贝构造需内联, 否则跨 DLL 边界崩溃 (0x8)
        {
            GenTaskSubType sub_type = GenTaskSubType::UNKNOWN; // 任务类型 (调用者需过滤)
            std::string prompt; // 文本提示词
            std::string negative_prompt; // 反向提示词
            int polygon_limit = 0; // 面数限制
            int texture_size = 0; // 纹理分辨率
            int provider_id = 0; // 供应商类型 (默认 0, 前端根据服务商列表选择)
            std::string model_version; // 模型版本 (字符串: 服务端可能新增版本)
            std::string upload_file_key; // 已上传文件的 key。前端先调用 UploadFile 上传本地素材
            // (图片/模型), 服务端返回 upload_file_key 后填入此处。
            // IMAGE_TO_MODEL / TEXTURE_MODEL 等需要输入素材时必填,
            // TEXT_TO_MODEL 等纯文字任务保持空字符串。

            // rapidjson 序列化 (对标 ATOptions::WriteToJson / ParseJson)
            void WriteToJson(rapidjson::Value& metadata, rapidjson::Document& document) const
            {
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                if (const char* s = ToString(sub_type))
                    metadata.AddMember("sub_type", rapidjson::Value(s, allocator), allocator);
                if (!String::StringIsNullOrBlank(prompt))
                    metadata.AddMember("prompt", rapidjson::Value(prompt.c_str(), allocator), allocator);
                if (!String::StringIsNullOrBlank(negative_prompt))
                    metadata.AddMember("negative_prompt", rapidjson::Value(negative_prompt.c_str(), allocator),
                                       allocator);
                if (polygon_limit != 0)
                    metadata.AddMember("polygon_limit", rapidjson::Value(polygon_limit), allocator);
                if (texture_size != 0)
                    metadata.AddMember("texture_size", rapidjson::Value(texture_size), allocator);
                if (provider_id != 0)
                    metadata.AddMember("provider_id", rapidjson::Value(provider_id), allocator);
                if (!String::StringIsNullOrBlank(model_version))
                    metadata.AddMember("model_version", rapidjson::Value(model_version.c_str(), allocator), allocator);
                if (!String::StringIsNullOrBlank(upload_file_key))
                    metadata.AddMember("upload_file_key", rapidjson::Value(upload_file_key.c_str(), allocator),
                                       allocator);
            }

            std::string ToJsonString() const
            {
                rapidjson::Document doc;
                doc.SetObject();
                WriteToJson(doc, doc);
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                doc.Accept(writer);
                return buffer.GetString();
            }

            void ParseJson(const rapidjson::Value& metadata)
            {
                if (metadata.HasMember("sub_type"))
                    sub_type = SubTypeFromString(metadata["sub_type"].GetString());
                if (metadata.HasMember("prompt"))
                    prompt = metadata["prompt"].GetString();
                if (metadata.HasMember("negative_prompt"))
                    negative_prompt = metadata["negative_prompt"].GetString();
                if (metadata.HasMember("polygon_limit"))
                    polygon_limit = metadata["polygon_limit"].GetInt();
                if (metadata.HasMember("texture_size"))
                    texture_size = metadata["texture_size"].GetInt();
                if (metadata.HasMember("provider_id"))
                    provider_id = metadata["provider_id"].GetInt();
                if (metadata.HasMember("model_version"))
                    model_version = metadata["model_version"].GetString();
                if (metadata.HasMember("upload_file_key"))
                    upload_file_key = metadata["upload_file_key"].GetString();
            }

            static GenTaskParams CreateFromJsonString(const std::string& jsonStr)
            {
                GenTaskParams p;
                if (String::StringIsNullOrBlank(jsonStr)) return p;
                rapidjson::Document doc;
                if (doc.Parse(jsonStr.c_str()).HasParseError()) return p;
                p.ParseJson(doc);
                return p;
            }
        };

        // ============================================================================

        // GenTaskOptions — 嵌入 BlockObject::Task_Info 的生成式任务配置
        // 对标 ATOptions at_options, 仅含生成参数
        // 任务类型判断由 Task_Info::block_task_category 负责
        // ============================================================================
        struct GenTaskOptions // 不加 AI3D_API — 同上
        {
            GenTaskParams gen_params; // 生成参数

            // rapidjson 序列化 (委托 gen_params, 对标 ATOptions::WriteToJson / ParseJson)
            void WriteToJson(rapidjson::Value& metadata, rapidjson::Document& document) const
            {
                gen_params.WriteToJson(metadata, document);
            }

            void ParseJson(const rapidjson::Value& metadata)
            {
                gen_params.ParseJson(metadata);
            }
        };

        // ============================================================================
        // blk_generation_info_s — 生成式任务结果元数据
        // 对标 blk_reconst_production_info_s, 存储在 Block 的 Task_Info 中, 供前端目录树展示
        // status / sub_type 存 int (避免 Core→Util 依赖), 读取后由调用者转枚举
        // ============================================================================
        struct blk_generation_info_s // 不加 AI3D_API — 同上, vector 拷贝会触发每个元素的拷贝构造
        {
            int generation_id = -1; // 对标 production_t id_
            std::string task_uuid; // 任务唯一标识
            std::string job_name; // job 文件名
            int sub_type = -1; // GenTaskSubType 枚举值
            int status = -1; // GenTaskStatus 枚举值
            std::string result_url; // 结果文件 key (前端用此 key 下载)
            std::string result_path; // 结果完整路径 (前端下载后回填)
            std::string preview_url;
            std::string preview_path; // 预览完整路径 (前端下载后回填)
            std::string result_dir; // 结果下载目录
            std::string created_time; // 创建时间 "yyyyMMddhhmmss"
            // --- 积分摘要 (前端无需读 job 文件, Block 中直接获取) ---
            int consumed = 0; // 实际消耗 (GenTask: 后端不返回, 恒为 0; 重建式: SettlePoints 后填充)
            int refunded = 0; // 返还积分 (同上)
            int total_balance = 0; // 总积分余额 (query/submit 均返回)
            int available_points = 0; // 可用积分 (query/submit 均返回)

            void CreateJson(rapidjson::Value& value, rapidjson::Document& doc) const
            {
                auto& allocator = doc.GetAllocator();
                value.AddMember("generation_id", rapidjson::Value(generation_id), allocator);
                value.AddMember("task_uuid", rapidjson::Value(task_uuid.c_str(), allocator), allocator);
                value.AddMember("job_name", rapidjson::Value(job_name.c_str(), allocator), allocator);
                value.AddMember("sub_type", rapidjson::Value(sub_type), allocator);
                value.AddMember("status", rapidjson::Value(status), allocator);
                if (!result_url.empty())
                    value.AddMember("result_url", rapidjson::Value(result_url.c_str(), allocator), allocator);
                if (!preview_url.empty())
                    value.AddMember("preview_url", rapidjson::Value(preview_url.c_str(), allocator), allocator);
                if (!result_path.empty())
                    value.AddMember("result_path", rapidjson::Value(result_path.c_str(), allocator), allocator);
                if (!preview_path.empty())
                    value.AddMember("preview_path", rapidjson::Value(preview_path.c_str(), allocator), allocator);
                if (!result_dir.empty())
                    value.AddMember("result_dir", rapidjson::Value(result_dir.c_str(), allocator), allocator);
                if (!created_time.empty())
                    value.AddMember("created_time", rapidjson::Value(created_time.c_str(), allocator), allocator);
                value.AddMember("consumed", rapidjson::Value(consumed), allocator);
                value.AddMember("refunded", rapidjson::Value(refunded), allocator);
                value.AddMember("total_balance", rapidjson::Value(total_balance), allocator);
                value.AddMember("available_points", rapidjson::Value(available_points), allocator);
            }

            void ParseJson(const rapidjson::Value& value)
            {
                if (value.HasMember("generation_id")) generation_id = value["generation_id"].GetInt();
                if (value.HasMember("task_uuid")) task_uuid = value["task_uuid"].GetString();
                if (value.HasMember("job_name")) job_name = value["job_name"].GetString();
                if (value.HasMember("sub_type")) sub_type = value["sub_type"].GetInt();
                if (value.HasMember("status")) status = value["status"].GetInt();
                if (value.HasMember("result_url")) result_url = value["result_url"].GetString();
                if (value.HasMember("preview_url")) preview_url = value["preview_url"].GetString();
                if (value.HasMember("created_time")) created_time = value["created_time"].GetString();
                if (value.HasMember("result_path")) result_path = value["result_path"].GetString();
                if (value.HasMember("preview_path")) preview_path = value["preview_path"].GetString();
                if (value.HasMember("result_dir")) result_dir = value["result_dir"].GetString();
                if (value.HasMember("consumed")) consumed = value["consumed"].GetInt();
                if (value.HasMember("refunded")) refunded = value["refunded"].GetInt();
                if (value.HasMember("total_balance")) total_balance = value["total_balance"].GetInt();
                if (value.HasMember("available_points")) available_points = value["available_points"].GetInt();
            }
        };
    } // namespace CORE
} // namespace AI3D
