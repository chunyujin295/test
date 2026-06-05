#pragma once

#include <Core/Rapidjson.h>
#include <string>
#include <Constants.h>
#include <Core/String.h>

namespace AI3D
{
    namespace CORE
    {
        enum class GenTaskStatus
        {
            UNKNOWN = -1,
            IDLE,
            PENDING,
            IN_PROGRESS,
            COMPLETED,
            FAILED,
            CANCELED,
        };

        enum class GenTaskSubType
        {
            UNKNOWN = -1,
            TEXT_TO_MODEL,
            TEXT_TO_MESH,
            IMAGE_TO_MODEL,
            IMAGE_TO_MESH,
            // TEXTURE_MODEL,
            // TEXT_TO_TEXTURE,
            // MODEL_PREVIEW_RENDER,
            // MODEL_REMESH,
            // CONVERT_MODEL_FORMAT,
            // IMAGE_GENERATION,
        };

        inline const char* ToString(GenTaskSubType st)
        {
            switch (st)
            {
            case GenTaskSubType::TEXT_TO_MODEL:
                return "text-to-model";
            case GenTaskSubType::TEXT_TO_MESH:
                return "text-to-mesh";
            case GenTaskSubType::IMAGE_TO_MODEL:
                return "image-to-model";
            case GenTaskSubType::IMAGE_TO_MESH:
                return "image-to-mesh";
            default:
                return nullptr;
            }
        }

        inline GenTaskSubType SubTypeFromString(const std::string& s)
        {
            if (s == "text-to-model")
                return GenTaskSubType::TEXT_TO_MODEL;
            if (s == "text-to-mesh")
                return GenTaskSubType::TEXT_TO_MESH;
            if (s == "image-to-model")
                return GenTaskSubType::IMAGE_TO_MODEL;
            if (s == "image-to-mesh")
                return GenTaskSubType::IMAGE_TO_MESH;
            return GenTaskSubType::UNKNOWN;
        }

        struct GenTaskParams
        {
            GenTaskSubType sub_type = GenTaskSubType::UNKNOWN;
            std::string prompt;
            std::string negative_prompt;
            int polygon_limit = 0;
            int texture_size = 0;
            std::string model_version;
            std::string file_key;

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
                if (!String::StringIsNullOrBlank(model_version))
                    metadata.AddMember("model_version", rapidjson::Value(model_version.c_str(), allocator), allocator);
                if (!String::StringIsNullOrBlank(file_key))
                    metadata.AddMember("file_key", rapidjson::Value(file_key.c_str(), allocator), allocator);
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
                if (metadata.HasMember("model_version"))
                    model_version = metadata["model_version"].GetString();
                if (metadata.HasMember("file_key"))
                    file_key = metadata["file_key"].GetString();
            }

            static GenTaskParams CreateFromJsonString(const std::string& jsonStr)
            {
                GenTaskParams p;
                if (String::StringIsNullOrBlank(jsonStr))
                    return p;
                rapidjson::Document doc;
                if (doc.Parse(jsonStr.c_str()).HasParseError())
                    return p;
                p.ParseJson(doc);
                return p;
            }
        };

        struct GenTaskOptions
        {
            GenTaskParams gen_params;

            void WriteToJson(rapidjson::Value& metadata, rapidjson::Document& document) const
            {
                gen_params.WriteToJson(metadata, document);
            }

            void ParseJson(const rapidjson::Value& metadata)
            {
                gen_params.ParseJson(metadata);
            }
        };

        struct blk_generation_info_s
        {
            std::string task_uuid;
            std::string job_name;
            int sub_type = -1;
            int status = -1;
            std::string result_url;
            std::string preview_url;
            std::string result_path;
            std::string preview_path;
            std::string created_time;

            void CreateJson(rapidjson::Value& value, rapidjson::Document& doc) const
            {
                auto& allocator = doc.GetAllocator();
                value.AddMember("task_uuid", rapidjson::Value(task_uuid.c_str(), allocator), allocator);
                value.AddMember("job_name", rapidjson::Value(job_name.c_str(), allocator), allocator);
                value.AddMember("sub_type", rapidjson::Value(sub_type), allocator);
                value.AddMember("status", rapidjson::Value(status), allocator);
                if (!String::StringIsNullOrBlank(preview_url))
                    value.AddMember("preview_url", rapidjson::Value(preview_url.c_str(), allocator), allocator);
                if (!String::StringIsNullOrBlank(result_url))
                    value.AddMember("result_url", rapidjson::Value(result_url.c_str(), allocator), allocator);
                if (!String::StringIsNullOrBlank(preview_path))
                    value.AddMember("preview_path", rapidjson::Value(preview_path.c_str(), allocator), allocator);
                if (!String::StringIsNullOrBlank(result_path))
                    value.AddMember("result_path", rapidjson::Value(result_path.c_str(), allocator), allocator);
                if (!String::StringIsNullOrBlank(created_time))
                    value.AddMember("created_time", rapidjson::Value(created_time.c_str(), allocator), allocator);
            }

            void ParseJson(const rapidjson::Value& value)
            {
                if (value.HasMember("task_uuid"))
                    task_uuid = value["task_uuid"].GetString();
                if (value.HasMember("job_name"))
                    job_name = value["job_name"].GetString();
                if (value.HasMember("sub_type"))
                    sub_type = value["sub_type"].GetInt();
                if (value.HasMember("status"))
                    status = value["status"].GetInt();
                if (value.HasMember("preview_url"))
                    preview_url = value["preview_url"].GetString();
                if (value.HasMember("result_url"))
                    result_url = value["result_url"].GetString();
                if (value.HasMember("preview_path"))
                    preview_path = value["preview_path"].GetString();
                if (value.HasMember("result_path"))
                    result_path = value["result_path"].GetString();
                if (value.HasMember("created_time"))
                    created_time = value["created_time"].GetString();
            }
        };
    };
}
