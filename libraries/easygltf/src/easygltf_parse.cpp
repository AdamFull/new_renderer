#include <easygltf_base.h>

namespace parsing
{
    template<class _Ty>
    inline void parse_from(const std::string& name, const nlohmann::json& json, _Ty& type, bool required = false)
    {
        if (auto obj = json.find(name); obj != json.end())
            type = obj->get<_Ty>();
        else
            if (required)
                throw std::runtime_error("Required argument was not found");
    }

    template<class _Ty>
    inline void serialize_to(const std::string& name, nlohmann::json& json, const _Ty& type, bool condition)
    {
        if (condition)
            json[name] = type;
    }

    template<class _Ty>
    inline void parse_extensions_and_extras(const nlohmann::json& json, _Ty& type)
    {
        parse_from("extensions", json, type.extensions);
        parse_from("extras", json, type.extras);
    }

    template<class _Ty>
    inline void serialize_extensions_and_extras(nlohmann::json& json, const _Ty& type)
    {
        serialize_to("extensions", json, type.extensions, !type.extensions.empty());
        serialize_to("extras", json, type.extras, !type.extras.empty());
    }

    template<class _Namespace, class _Ty>
    void parse_extension(const std::string& name, const nlohmann::json& json, _Ty& type)
    {
        if (name == _Namespace::name)
            type = json.get<_Ty>();
    }

    template<class _Extension, class _Ty>
    void serialize_extension(std::unordered_map<std::string, nlohmann::json>& extensions, const _Ty& type)
    {
        if (type != _Ty{})
            extensions.emplace(_Extension::name, nlohmann::json(type));
    }

    
    void serialize_used_extensions(std::unordered_set<std::string>& extensionsUsed, const nlohmann::json& json)
    {
        if (json.is_object())
        {
            for (auto it = json.begin(); it != json.end(); ++it)
            {
                if (it.key() == "extensions" && it->is_object()) 
                {
                    for (auto ext_it = it->begin(); ext_it != it->end(); ++ext_it) 
                        extensionsUsed.emplace(ext_it.key());
                }
                else if (it->is_object() || it->is_array())
                    serialize_used_extensions(extensionsUsed , *it);
            }
        }
        else if (json.is_array()) 
        {
            for (auto& element : json)
            {
                if (element.is_object() || element.is_array())
                    serialize_used_extensions(extensionsUsed, element);
            }
        }
    }
    //template<class _Ty>
    //void apply_used_extensions(easygltf::FGLTFRoot::)
}

namespace easygltf
{
    // ----------------- Extensions -----------------
    namespace extensions
    {
        // ----------------- EXT_materials_displacement -----------------
        // ----------------- MaterialsDisplacement -----------------
        void to_json(nlohmann::json& json, const EXT_materials_displacement::MaterialsDisplacement& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("displacementTexture", json, type.displacementTexture, type.displacementTexture != TextureInfo{});
            parsing::serialize_to("tessellationFactor", json, type.tessellationFactor, !compare_double(type.tessellationFactor, base::constants::done));
            parsing::serialize_to("displacementStrength", json, type.displacementStrength, !compare_double(type.displacementStrength, base::constants::done));

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, EXT_materials_displacement::MaterialsDisplacement& type)
        {
            parsing::parse_from("displacementTexture", json, type.displacementTexture);
            parsing::parse_from("tessellationFactor", json, type.tessellationFactor);
            parsing::parse_from("displacementStrength", json, type.displacementStrength);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- EXT_lights_shadows -----------------
        // ----------------- CastShadows -----------------
        void to_json(nlohmann::json& json, const EXT_lights_shadows::CastShadows& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("castShadows", json, type.castShadows, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, EXT_lights_shadows::CastShadows& type)
        {
            parsing::parse_from("castShadows", json, type.castShadows, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_audio -----------------
        // ----------------- AudioNode -----------------
        void to_json(nlohmann::json& json, const KHR_audio::AudioNode& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("emitter", json, type.emitter, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_audio::AudioNode& type)
        {
            parsing::parse_from("emitter", json, type.emitter, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- AudioScene -----------------
        void to_json(nlohmann::json& json, const KHR_audio::AudioScene& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("emitters", json, type.emitters, true);

            parsing::serialize_extensions_and_extras(json, type);
        }
        
        void from_json(const nlohmann::json& json, KHR_audio::AudioScene& type)
        {
            parsing::parse_from("emitters", json, type.emitters, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- PositionalEmitter -----------------
        void to_json(nlohmann::json& json, const KHR_audio::PositionalEmitter& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("coneInnerAngle", json, type.coneInnerAngle, !compare_double(type.coneInnerAngle, KHR_audio::constants::cone_angle_default));
            parsing::serialize_to("coneOuterAngle", json, type.coneOuterAngle, !compare_double(type.coneOuterAngle, KHR_audio::constants::cone_angle_default));
            parsing::serialize_to("coneOuterGain", json, type.coneOuterGain, !compare_double(type.coneOuterGain, KHR_audio::constants::outer_gain_default));
            parsing::serialize_to("maxDistance", json, type.maxDistance, !compare_double(type.maxDistance, KHR_audio::constants::max_distance_default));
            parsing::serialize_to("refDistance", json, type.refDistance, !compare_double(type.refDistance, KHR_audio::constants::ref_distance_default));
            parsing::serialize_to("rolloffFactor", json, type.rolloffFactor, !compare_double(type.rolloffFactor, KHR_audio::constants::rolloff_factor_default));

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_audio::PositionalEmitter& type)
        {
            parsing::parse_from("coneInnerAngle", json, type.coneInnerAngle);
            parsing::parse_from("coneOuterAngle", json, type.coneOuterAngle);
            parsing::parse_from("coneOuterGain", json, type.coneOuterGain);
            parsing::parse_from("maxDistance", json, type.maxDistance);
            parsing::parse_from("refDistance", json, type.refDistance);
            parsing::parse_from("rolloffFactor", json, type.rolloffFactor);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- AudioEmitter -----------------
        void to_json(nlohmann::json& json, const KHR_audio::AudioEmitter& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("name", json, type.name, !type.name.empty());
            parsing::serialize_to("gain", json, type.gain, !compare_double(type.gain, base::constants::done));
            parsing::serialize_to("loop", json, type.loop, type.loop);
            parsing::serialize_to("playing", json, type.playing, type.playing);
            parsing::serialize_to("type", json, type.type, type.type != KHR_audio::enumerators::source_type::global);
            parsing::serialize_to("distanceModel", json, type.distanceModel, type.distanceModel != KHR_audio::enumerators::distance_model::inverse);

            if(type.type == KHR_audio::enumerators::source_type::positional)
                parsing::serialize_to("positional", json, type.positional, type.positional != KHR_audio::PositionalEmitter{});

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_audio::AudioEmitter& type)
        {
            parsing::parse_from("name", json, type.name);
            parsing::parse_from("gain", json, type.gain);
            parsing::parse_from("loop", json, type.loop);
            parsing::parse_from("playing", json, type.playing);
            parsing::parse_from("type", json, type.type);
            parsing::parse_from("distanceModel", json, type.distanceModel);

            if (type.type == KHR_audio::enumerators::source_type::positional)
                parsing::parse_from("positional", json, type.positional);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- AudioSource -----------------
        void to_json(nlohmann::json& json, const KHR_audio::AudioSource& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("name", json, type.name, !type.name.empty());

            if (!type.uri.empty())
                parsing::serialize_to("uri", json, type.uri, true);
            else
            {
                parsing::serialize_to("bufferView", json, type.bufferView, true);
                parsing::serialize_to("mimeType", json, type.mimeType, true);
            }

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_audio::AudioSource& type)
        {
            parsing::parse_from("name", json, type.name);
            parsing::parse_from("uri", json, type.uri);

            if (type.uri.empty())
            {
                parsing::parse_from("bufferView", json, type.bufferView, true);
                parsing::parse_from("mimeType", json, type.mimeType, true);
            }

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- Audio -----------------
        void to_json(nlohmann::json& json, const KHR_audio::Audio& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("emitters", json, type.emitters, true);
            parsing::serialize_to("sources", json, type.sources, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_audio::Audio& type)
        {
            parsing::parse_from("emitters", json, type.emitters);
            parsing::parse_from("sources", json, type.sources);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_draco_mesh_compression -----------------
        // ----------------- DracoMeshCompression -----------------
        void to_json(nlohmann::json& json, const KHR_draco_mesh_compression::DracoMeshCompression& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("bufferView", json, type.bufferView, true);
            parsing::serialize_to("attributes", json, type.attributes, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_draco_mesh_compression::DracoMeshCompression& type)
        {
            parsing::parse_from("bufferView", json, type.bufferView, true);
            parsing::parse_from("attributes", json, type.attributes, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_lights_punctual -----------------
        // ----------------- SpotLight -----------------
        void to_json(nlohmann::json& json, const KHR_lights_punctual::SpotLight& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("innerConeAngle", json, type.innerConeAngle, !compare_double(type.innerConeAngle, KHR_lights_punctual::constants::inner_default));
            parsing::serialize_to("outerConeAngle", json, type.outerConeAngle, !compare_double(type.outerConeAngle, KHR_lights_punctual::constants::outer_default));

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_lights_punctual::SpotLight& type)
        {
            parsing::parse_from("innerConeAngle", json, type.innerConeAngle);
            parsing::parse_from("outerConeAngle", json, type.outerConeAngle);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- Light -----------------
        void to_json(nlohmann::json& json, const KHR_lights_punctual::Light& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("name", json, type.name, !type.name.empty());
            parsing::serialize_to("color", json, type.color, !compare_dvec3(type.color, base::constants::dvec3ones));
            parsing::serialize_to("intensity", json, type.color, !compare_double(type.intensity, base::constants::done));
            parsing::serialize_to("spot", json, type.spot, type.type == KHR_lights_punctual::enumerators::light_type::spot);
            parsing::serialize_to("type", json, type.type, true);
            parsing::serialize_to("range", json, type.range, !compare_double(type.range, base::constants::dzero));

            parsing::serialize_extension<extensions::EXT_lights_shadows>(type.extensions, type.extLightShadows);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_lights_punctual::Light& type)
        {
            parsing::parse_from("name", json, type.name);
            parsing::parse_from("color", json, type.color);
            parsing::parse_from("intensity", json, type.intensity);
            parsing::parse_from("type", json, type.type, true);

            if(type.type == KHR_lights_punctual::enumerators::light_type::spot)
                parsing::parse_from("spot", json, type.spot);

            parsing::parse_from("range", json, type.range);

            parsing::parse_extensions_and_extras(json, type);

            for (auto& [name, ext] : type.extensions)
                parsing::parse_extension<extensions::EXT_lights_shadows>(name, ext, type.extLightShadows);
        }

        // ----------------- LightsPunctual -----------------
        void to_json(nlohmann::json& json, const KHR_lights_punctual::LightsPunctual& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("lights", json, type.lights, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_lights_punctual::LightsPunctual& type)
        {
            parsing::parse_from("lights", json, type.lights, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- LightsPunctualNode -----------------
        void to_json(nlohmann::json& json, const KHR_lights_punctual::LightsPunctualNode& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("light", json, type.light, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_lights_punctual::LightsPunctualNode& type)
        {
            parsing::parse_from("light", json, type.light, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_anisotropy -----------------
        // ----------------- MaterialsAnisotropy -----------------
        void to_json(nlohmann::json& json, const KHR_materials_anisotropy::MaterialsAnisotropy& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("anisotropyStrength", json, type.anisotropyStrength, !compare_double(type.anisotropyStrength, KHR_materials_anisotropy::constants::anisotropy_strength_default));
            parsing::serialize_to("anisotropyRotation", json, type.anisotropyRotation, !compare_double(type.anisotropyRotation, base::constants::dzero));
            parsing::serialize_to("anisotropyTexture", json, type.anisotropyTexture, type.anisotropyTexture != TextureInfo{});

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_anisotropy::MaterialsAnisotropy& type)
        {
            parsing::parse_from("anisotropyStrength", json, type.anisotropyStrength);
            parsing::parse_from("anisotropyRotation", json, type.anisotropyRotation);
            parsing::parse_from("anisotropyTexture", json, type.anisotropyTexture);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_clearcoat -----------------
        // ----------------- MaterialClearcoat -----------------
        void to_json(nlohmann::json& json, const KHR_materials_clearcoat::MaterialClearcoat& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("clearcoatFactor", json, type.clearcoatFactor, !compare_double(type.clearcoatFactor, base::constants::dzero));
            parsing::serialize_to("clearcoatTexture", json, type.clearcoatTexture, type.clearcoatTexture != TextureInfo{});
            parsing::serialize_to("clearcoatRoughnessFactor", json, type.clearcoatRoughnessFactor, !compare_double(type.clearcoatRoughnessFactor, base::constants::dzero));
            parsing::serialize_to("clearcoatRoughnessTexture", json, type.clearcoatRoughnessTexture, type.clearcoatRoughnessTexture != TextureInfo{});
            parsing::serialize_to("clearcoatNormalTexture", json, type.clearcoatNormalTexture, type.clearcoatNormalTexture != NormalTexture{});

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_clearcoat::MaterialClearcoat& type)
        {
            parsing::parse_from("clearcoatFactor", json, type.clearcoatFactor);
            parsing::parse_from("clearcoatTexture", json, type.clearcoatTexture);
            parsing::parse_from("clearcoatRoughnessFactor", json, type.clearcoatRoughnessFactor);
            parsing::parse_from("clearcoatRoughnessTexture", json, type.clearcoatRoughnessTexture);
            parsing::parse_from("clearcoatNormalTexture", json, type.clearcoatNormalTexture);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_emissive_strength -----------------
        // ----------------- MaterialEmissiveStrength -----------------
        void to_json(nlohmann::json& json, const KHR_materials_emissive_strength::MaterialEmissiveStrength& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("emissiveStrength", json, type.emissiveStrength, !compare_double(type.emissiveStrength, base::constants::done));

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_emissive_strength::MaterialEmissiveStrength& type)
        {
            parsing::parse_from("emissiveStrength", json, type.emissiveStrength);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_ior -----------------
        // ----------------- MaterialIor -----------------
        void to_json(nlohmann::json& json, const KHR_materials_ior::MaterialIor& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("ior", json, type.ior, !compare_double(type.ior, KHR_materials_ior::constants::ior_default));

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_ior::MaterialIor& type)
        {
            parsing::parse_from("ior", json, type.ior);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_iridescence -----------------
        // ----------------- MaterialIridescence -----------------
        void to_json(nlohmann::json& json, const KHR_materials_iridescence::MaterialIridescence& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("iridescenceFactor", json, type.iridescenceFactor, !compare_double(type.iridescenceFactor, KHR_materials_iridescence::constants::factor_default));
            parsing::serialize_to("iridescenceTexture", json, type.iridescenceTexture, type.iridescenceTexture != TextureInfo{});
            parsing::serialize_to("iridescenceIor", json, type.iridescenceIor, !compare_double(type.iridescenceIor, KHR_materials_iridescence::constants::ior_default));
            parsing::serialize_to("iridescenceThicknessMinimum", json, type.iridescenceThicknessMinimum, !compare_double(type.iridescenceThicknessMinimum, KHR_materials_iridescence::constants::thickness_min_default));
            parsing::serialize_to("iridescenceThicknessMaximum", json, type.iridescenceThicknessMaximum, !compare_double(type.iridescenceThicknessMaximum, KHR_materials_iridescence::constants::thickness_max_default));
            parsing::serialize_to("iridescenceThicknessTexture", json, type.iridescenceThicknessTexture, type.iridescenceThicknessTexture != TextureInfo{});

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_iridescence::MaterialIridescence& type)
        {
            parsing::parse_from("iridescenceFactor", json, type.iridescenceFactor);
            parsing::parse_from("iridescenceTexture", json, type.iridescenceTexture);
            parsing::parse_from("iridescenceIor", json, type.iridescenceIor);
            parsing::parse_from("iridescenceThicknessMinimum", json, type.iridescenceThicknessMinimum);
            parsing::parse_from("iridescenceThicknessMaximum", json, type.iridescenceThicknessMaximum);
            parsing::parse_from("iridescenceThicknessTexture", json, type.iridescenceThicknessTexture);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_sheen -----------------
        // ----------------- MaterialSheen -----------------
        void to_json(nlohmann::json& json, const KHR_materials_sheen::MaterialSheen& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("sheenColorFactor", json, type.sheenColorFactor, !compare_dvec3(type.sheenColorFactor, base::constants::dvec3zeros));
            parsing::serialize_to("sheenColorTexture", json, type.sheenColorTexture, type.sheenColorTexture != TextureInfo{});
            parsing::serialize_to("sheenRoughnessFactor", json, type.sheenRoughnessFactor, !compare_double(type.sheenRoughnessFactor, base::constants::dzero));
            parsing::serialize_to("sheenRoughnessTexture", json, type.sheenRoughnessTexture, type.sheenRoughnessTexture != TextureInfo{});

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_sheen::MaterialSheen& type)
        {
            parsing::parse_from("sheenColorFactor", json, type.sheenColorFactor);
            parsing::parse_from("sheenColorTexture", json, type.sheenColorTexture);
            parsing::parse_from("sheenRoughnessFactor", json, type.sheenRoughnessFactor);
            parsing::parse_from("sheenRoughnessTexture", json, type.sheenRoughnessTexture);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_specular -----------------
        // ----------------- MaterialSpecular -----------------
        void to_json(nlohmann::json& json, const KHR_materials_specular::MaterialSpecular& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("specularFactor", json, type.specularFactor, !compare_double(type.specularFactor, KHR_materials_specular::constants::factor_default));
            parsing::serialize_to("specularTexture", json, type.specularTexture, type.specularTexture != TextureInfo{});
            parsing::serialize_to("specularColorFactor", json, type.specularColorFactor, !compare_dvec3(type.specularColorFactor, base::constants::dvec3ones));
            parsing::serialize_to("specularColorTexture", json, type.specularColorTexture, type.specularColorTexture != TextureInfo{});

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_specular::MaterialSpecular& type)
        {
            parsing::parse_from("specularFactor", json, type.specularFactor);
            parsing::parse_from("specularTexture", json, type.specularTexture);
            parsing::parse_from("specularColorFactor", json, type.specularColorFactor);
            parsing::parse_from("specularColorTexture", json, type.specularColorTexture);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_transmission -----------------
        // ----------------- MaterialTransmission -----------------
        void to_json(nlohmann::json& json, const KHR_materials_transmission::MaterialTransmission& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("transmissionFactor", json, type.transmissionFactor, !compare_double(type.transmissionFactor, KHR_materials_transmission::constants::factor_default));
            parsing::serialize_to("transmissionTexture", json, type.transmissionTexture, type.transmissionTexture != TextureInfo{});

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_transmission::MaterialTransmission& type)
        {
            parsing::parse_from("transmissionFactor", json, type.transmissionFactor);
            parsing::parse_from("transmissionTexture", json, type.transmissionTexture);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_unlit -----------------
        // ----------------- MaterialUnlit -----------------
        void to_json(nlohmann::json& json, const KHR_materials_unlit::MaterialUnlit& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_unlit::MaterialUnlit& type)
        {
            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_variants -----------------
        // ----------------- Variant -----------------
        void to_json(nlohmann::json& json, const KHR_materials_variants::Variant& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("name", json, type.name, !type.name.empty());

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_variants::Variant& type)
        {
            parsing::parse_from("name", json, type.name);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- MaterialVariants -----------------
        void to_json(nlohmann::json& json, const KHR_materials_variants::MaterialVariants& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("variants", json, type.variants, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_variants::MaterialVariants& type)
        {
            parsing::parse_from("variants", json, type.variants, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- Mapping -----------------
        void to_json(nlohmann::json& json, const KHR_materials_variants::Mapping& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("name", json, type.name, !type.name.empty());
            parsing::serialize_to("variants", json, type.variants, true);
            parsing::serialize_to("material", json, type.material, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_variants::Mapping& type)
        {
            parsing::parse_from("name", json, type.name);
            parsing::parse_from("variants", json, type.variants, true);
            parsing::parse_from("material", json, type.material, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- MaretialVariant -----------------
        void to_json(nlohmann::json& json, const KHR_materials_variants::MaretialVariant& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("mappings", json, type.mappings, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_variants::MaretialVariant& type)
        {
            parsing::parse_from("mappings", json, type.mappings, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_materials_volume -----------------
        // ----------------- MaterialVolume -----------------
        void to_json(nlohmann::json& json, const KHR_materials_volume::MaterialVolume& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("thicknessFactor", json, type.thicknessFactor, !compare_double(type.thicknessFactor, base::constants::dzero));
            parsing::serialize_to("thicknessTexture", json, type.thicknessTexture, type.thicknessTexture != TextureInfo{});
            parsing::serialize_to("attenuationDistance", json, type.attenuationDistance, !compare_double(type.attenuationDistance, base::constants::dzero));
            parsing::serialize_to("attenuationColor", json, type.attenuationColor, !compare_dvec3(type.attenuationColor, base::constants::dvec3ones));

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_materials_volume::MaterialVolume& type)
        {
            parsing::parse_from("thicknessFactor", json, type.thicknessFactor);
            parsing::parse_from("thicknessTexture", json, type.thicknessTexture);
            parsing::parse_from("attenuationDistance", json, type.attenuationDistance);
            parsing::parse_from("attenuationColor", json, type.attenuationColor);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_texture_basisu -----------------
        // ----------------- TextureBasisU -----------------
        void to_json(nlohmann::json& json, const KHR_texture_basisu::TextureBasisU& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("source", json, type.source, true);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_texture_basisu::TextureBasisU& type)
        {
            parsing::parse_from("source", json, type.source, true);

            parsing::parse_extensions_and_extras(json, type);
        }

        // ----------------- KHR_texture_transform -----------------
        // ----------------- TextureTransform -----------------
        void to_json(nlohmann::json& json, const KHR_texture_transform::TextureTransform& type)
        {
            json = nlohmann::json::object();

            parsing::serialize_to("offset", json, type.offset, !compare_dvec2(type.offset, base::constants::dvec2zeros));
            parsing::serialize_to("rotation", json, type.rotation, !compare_double(type.rotation, base::constants::dzero));
            parsing::serialize_to("scale", json, type.scale, !compare_dvec2(type.scale, base::constants::dvec2ones));
            parsing::serialize_to("texCoord", json, type.texCoord, type.texCoord != base::constants::not_set);

            parsing::serialize_extensions_and_extras(json, type);
        }

        void from_json(const nlohmann::json& json, KHR_texture_transform::TextureTransform& type)
        {
            parsing::parse_from("offset", json, type.offset);
            parsing::parse_from("rotation", json, type.rotation);
            parsing::parse_from("scale", json, type.scale);
            parsing::parse_from("texCoord", json, type.texCoord);

            parsing::parse_extensions_and_extras(json, type);
        }
    }

    // ----------------- SparseAccessorIndices -----------------
    void to_json(nlohmann::json& json, const SparseAccessorIndices& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("bufferView", json, type.bufferView, true);
        parsing::serialize_to("byteOffset", json, type.byteOffset, type.byteOffset != 0ull);
        parsing::serialize_to("componentType", json, type.componentType, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, SparseAccessorIndices& type)
    {
        parsing::parse_from("bufferView", json, type.bufferView, true);
        parsing::parse_from("byteOffset", json, type.byteOffset);
        parsing::parse_from("componentType", json, type.componentType, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- SparseAccessorValues -----------------
    void to_json(nlohmann::json& json, const SparseAccessorValues& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("bufferView", json, type.bufferView, true);
        parsing::serialize_to("byteOffset", json, type.byteOffset, type.byteOffset != 0ull);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, SparseAccessorValues& type)
    {
        parsing::parse_from("bufferView", json, type.bufferView, true);
        parsing::parse_from("byteOffset", json, type.byteOffset);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- SparseAccessor -----------------
    void to_json(nlohmann::json& json, const SparseAccessor& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("count", json, type.count, true);
        parsing::serialize_to("indices", json, type.indices, true);
        parsing::serialize_to("values", json, type.values, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, SparseAccessor& type)
    {
        parsing::parse_from("count", json, type.count, true);
        parsing::parse_from("indices", json, type.indices, true);
        parsing::parse_from("values", json, type.values, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Accessor -----------------
    void to_json(nlohmann::json& json, const Accessor& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("bufferView", json, type.bufferView, type.bufferView > base::constants::not_set);
        parsing::serialize_to("byteOffset", json, type.byteOffset, type.byteOffset != 0ull);
        parsing::serialize_to("count", json, type.count, true);
        parsing::serialize_to("normalized", json, type.normalized, type.normalized);
        parsing::serialize_to("componentType", json, type.componentType, true);
        parsing::serialize_to("type", json, type.type, true);

        parsing::serialize_to("min", json, type.min, true);
        parsing::serialize_to("max", json, type.max, true);

        parsing::serialize_to("sparse", json, type.sparse, type.sparse != SparseAccessor{});

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Accessor& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("bufferView", json, type.bufferView);
        parsing::parse_from("byteOffset", json, type.byteOffset);
        parsing::parse_from("count", json, type.count, true);
        parsing::parse_from("normalized", json, type.normalized);
        parsing::parse_from("componentType", json, type.componentType);
        parsing::parse_from("type", json, type.type);
        
        parsing::parse_from("min", json, type.min);
        parsing::parse_from("max", json, type.max);

        parsing::parse_from("sparse", json, type.sparse);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- AnimationTarget -----------------
    void to_json(nlohmann::json& json, const AnimationTarget& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("node", json, type.node, type.node > base::constants::not_set);
        parsing::serialize_to("path", json, type.path, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, AnimationTarget& type)
    {
        parsing::parse_from("node", json, type.node);
        parsing::parse_from("path", json, type.path, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- AnimationChannel -----------------
    void to_json(nlohmann::json& json, const AnimationChannel& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("sampler", json, type.sampler, true);
        parsing::serialize_to("target", json, type.target, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, AnimationChannel& type)
    {
        parsing::parse_from("sampler", json, type.sampler, true);
        parsing::parse_from("target", json, type.target, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- AnimationSampler -----------------
    void to_json(nlohmann::json& json, const AnimationSampler& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("input", json, type.input, true);
        parsing::serialize_to("output", json, type.output, true);
        parsing::serialize_to("interpolation", json, type.interpolation, type.interpolation != base::enumerators::interpolation::linear);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, AnimationSampler& type)
    {
        parsing::parse_from("input", json, type.input, true);
        parsing::parse_from("output", json, type.output, true);
        parsing::parse_from("interpolation", json, type.interpolation);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Animation -----------------
    void to_json(nlohmann::json& json, const Animation& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("channels", json, type.channels, true);
        parsing::serialize_to("samplers", json, type.samplers, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Animation& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("channels", json, type.channels, true);
        parsing::parse_from("samplers", json, type.samplers, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Asset -----------------
    void to_json(nlohmann::json& json, const Asset& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("version", json, type.version, !type.version.empty());
        parsing::serialize_to("generator", json, type.generator, !type.generator.empty());
        parsing::serialize_to("minVersion", json, type.minVersion, !type.minVersion.empty());
        parsing::serialize_to("copyright", json, type.copyright, !type.copyright.empty());

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Asset& type)
    {
        // Base
        parsing::parse_from("version", json, type.version, true);
        parsing::parse_from("generator", json, type.generator);
        parsing::parse_from("minVersion", json, type.minVersion);
        parsing::parse_from("copyright", json, type.copyright);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Buffer -----------------
    void to_json(nlohmann::json& json, const Buffer& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("uri", json, type.uri, !type.uri.empty());
        parsing::serialize_to("byteLength", json, type.byteLength, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Buffer& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("uri", json, type.uri);
        parsing::parse_from("byteLength", json, type.byteLength, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- BufferView -----------------
    void to_json(nlohmann::json& json, const BufferView& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("buffer", json, type.buffer, true);
        parsing::serialize_to("byteOffset", json, type.byteOffset, type.byteOffset != 0ull);
        parsing::serialize_to("byteLength", json, type.byteLength, true);
        parsing::serialize_to("byteStride", json, type.byteStride, type.byteStride != 0ull);
        parsing::serialize_to("target", json, type.target, type.target != 0);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, BufferView& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("buffer", json, type.buffer, true);
        parsing::parse_from("byteOffset", json, type.byteOffset);
        parsing::parse_from("byteLength", json, type.byteLength, true);
        parsing::parse_from("byteStride", json, type.byteStride);
        parsing::parse_from("target", json, type.target);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Perspective -----------------
    void to_json(nlohmann::json& json, const PerspectiveCamera& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("aspectRatio", json, type.aspectRatio, !compare_double(type.aspectRatio, base::constants::dzero));
        parsing::serialize_to("yfov", json, type.yfov, true);
        parsing::serialize_to("zfar", json, type.zfar, !compare_double(type.zfar, base::constants::dzero));
        parsing::serialize_to("znear", json, type.znear, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, PerspectiveCamera& type)
    {
        parsing::parse_from("aspectRatio", json, type.aspectRatio);
        parsing::parse_from("yfov", json, type.yfov, true);
        parsing::parse_from("zfar", json, type.zfar);
        parsing::parse_from("znear", json, type.znear, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Orthographic -----------------
    void to_json(nlohmann::json& json, const OrthographicCamera& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("xmag", json, type.xmag, true);
        parsing::serialize_to("ymag", json, type.ymag, true);
        parsing::serialize_to("zfar", json, type.zfar, true);
        parsing::serialize_to("znear", json, type.znear, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, OrthographicCamera& type)
    {
        parsing::parse_from("xmag", json, type.xmag, true);
        parsing::parse_from("ymag", json, type.ymag, true);
        parsing::parse_from("zfar", json, type.zfar, true);
        parsing::parse_from("znear", json, type.znear, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Camera -----------------
    void to_json(nlohmann::json& json, const Camera& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("type", json, type.type, type.type != base::enumerators::camera_type::perspective);

        if (type.type == "perspective")
            parsing::serialize_to("perspective", json, type.perspective, true);
        else
            parsing::serialize_to("orthographic", json, type.orthographic, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Camera& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("type", json, type.type);

        if (type.type == "perspective")
            parsing::parse_from("perspective", json, type.perspective, true);
        else
            parsing::parse_from("orthographic", json, type.orthographic, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Image -----------------
    void to_json(nlohmann::json& json, const Image& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());

        if (!type.uri.empty())
            parsing::serialize_to("uri", json, type.uri, true);
        else
        {
            parsing::serialize_to("bufferView", json, type.bufferView, true);
            parsing::serialize_to("mimeType", json, type.mimeType, true);
        }

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Image& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("uri", json, type.uri);

        if (type.uri.empty())
        {
            parsing::parse_from("bufferView", json, type.bufferView, true);
            parsing::parse_from("mimeType", json, type.mimeType, true);
        }

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- TextureInfo -----------------
    void to_json(nlohmann::json& json, const TextureInfo& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("index", json, type.index, true);
        parsing::serialize_to("texCoord", json, type.texCoord, type.texCoord > base::constants::not_set);

        // Serialize extensions
        parsing::serialize_extension<extensions::KHR_texture_transform>(type.extensions, type.khrTextureTransform);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, TextureInfo& type)
    {
        parsing::parse_from("index", json, type.index, true);
        parsing::parse_from("texCoord", json, type.texCoord);

        parsing::parse_extensions_and_extras(json, type);

        // Parse extensions
        for (auto& [name, ext] : type.extensions)
        {
            parsing::parse_extension<extensions::KHR_texture_transform>(name, ext, type.khrTextureTransform);
        }
    }

    // ----------------- TextureInfo -----------------
    void to_json(nlohmann::json& json, const CubemapTexture& type)
    {
        json = nlohmann::json::object();

        auto* baseClass = static_cast<const TextureInfo*>(&type);
        to_json(json, *baseClass);

        if (type.index == base::constants::not_set)
        {
            parsing::serialize_to("positiveX", json, type.positiveX, true);
            parsing::serialize_to("negativeX", json, type.negativeX, true);
            parsing::serialize_to("positiveY", json, type.positiveY, true);
            parsing::serialize_to("negativeY", json, type.negativeY, true);
            parsing::serialize_to("positiveZ", json, type.positiveZ, true);
            parsing::serialize_to("negativeZ", json, type.negativeZ, true);
        }
    }

    void from_json(const nlohmann::json& json, CubemapTexture& type)
    {
        auto* baseClass = static_cast<TextureInfo*>(&type);
        from_json(json, *baseClass);

        if (type.index == base::constants::not_set)
        {
            parsing::parse_from("positiveX", json, type.positiveX, true);
            parsing::parse_from("negativeX", json, type.negativeX, true);
            parsing::parse_from("positiveY", json, type.positiveY, true);
            parsing::parse_from("negativeY", json, type.negativeY, true);
            parsing::parse_from("positiveZ", json, type.positiveZ, true);
            parsing::parse_from("negativeZ", json, type.negativeZ, true);
        }
    }

    // ----------------- PbrMetallicRoughness -----------------
    void to_json(nlohmann::json& json, const PbrMetallicRoughness& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("baseColorFactor", json, type.baseColorFactor, !compare_dvec4(type.baseColorFactor, base::constants::dvec4ones));
        parsing::serialize_to("baseColorTexture", json, type.baseColorTexture, type.baseColorTexture != TextureInfo{});
        parsing::serialize_to("metallicFactor", json, type.metallicFactor, !compare_double(type.metallicFactor, base::constants::done));
        parsing::serialize_to("roughnessFactor", json, type.roughnessFactor, !compare_double(type.roughnessFactor, base::constants::done));
        parsing::serialize_to("metallicRoughnessTexture", json, type.metallicRoughnessTexture, type.metallicRoughnessTexture != TextureInfo{});

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, PbrMetallicRoughness& type)
    {
        parsing::parse_from("baseColorFactor", json, type.baseColorFactor);
        parsing::parse_from("baseColorTexture", json, type.baseColorTexture);
        parsing::parse_from("metallicFactor", json, type.metallicFactor);
        parsing::parse_from("roughnessFactor", json, type.roughnessFactor);
        parsing::parse_from("metallicRoughnessTexture", json, type.metallicRoughnessTexture);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- NormalTexture -----------------
    void to_json(nlohmann::json& json, const NormalTexture& type)
    {
        json = nlohmann::json::object();

        auto* baseClass = static_cast<const TextureInfo*>(&type);
        to_json(json, *baseClass);

        parsing::serialize_to("scale", json, type.scale, !compare_double(type.scale, base::constants::done));
    }

    void from_json(const nlohmann::json& json, NormalTexture& type)
    {
        auto* baseClass = static_cast<TextureInfo*>(&type);
        from_json(json, *baseClass);

        parsing::parse_from("scale", json, type.scale);
    }

    // ----------------- OcclusionTexture -----------------
    void to_json(nlohmann::json& json, const OcclusionTexture& type)
    {
        json = nlohmann::json::object();

        auto* baseClass = static_cast<const TextureInfo*>(&type);
        to_json(json, *baseClass);

        parsing::serialize_to("strength", json, type.strength, !compare_double(type.strength, base::constants::done));
    }

    void from_json(const nlohmann::json& json, OcclusionTexture& type)
    {
        auto* baseClass = static_cast<TextureInfo*>(&type);
        from_json(json, *baseClass);

        parsing::parse_from("strength", json, type.strength);
    }

    // ----------------- Material -----------------
    void to_json(nlohmann::json& json, const Material& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("pbrMetallicRoughness", json, type.pbrMetallicRoughness, type.pbrMetallicRoughness != PbrMetallicRoughness{});
        parsing::serialize_to("normalTexture", json, type.normalTexture, type.normalTexture != NormalTexture{});
        parsing::serialize_to("occlusionTexture", json, type.occlusionTexture, type.occlusionTexture != OcclusionTexture{});
        parsing::serialize_to("emissiveTexture", json, type.emissiveTexture, type.emissiveTexture != TextureInfo{});
        parsing::serialize_to("emissiveTFactor", json, type.emissiveFactor, !compare_dvec3(type.emissiveFactor, base::constants::dvec3zeros));
        parsing::serialize_to("alphaMode", json, type.alphaMode, type.alphaMode != base::enumerators::alpha_mode::opaque);
        parsing::serialize_to("alphaCutoff", json, type.alphaCutoff, !compare_double(type.alphaCutoff, 0.5));
        parsing::serialize_to("doubleSided", json, type.doubleSided, type.doubleSided);

        // Serialize extensions
        parsing::serialize_extension<extensions::EXT_materials_displacement>(type.extensions, type.extMaterialDisplacement);
        parsing::serialize_extension<extensions::KHR_materials_anisotropy>(type.extensions, type.khrMaterialAnisotropy);
        parsing::serialize_extension<extensions::KHR_materials_clearcoat>(type.extensions, type.khrMaterialClearcoat);
        parsing::serialize_extension<extensions::KHR_materials_emissive_strength>(type.extensions, type.khrMaterialEmissiveStrength);
        parsing::serialize_extension<extensions::KHR_materials_ior>(type.extensions, type.khrMaterialIor);
        parsing::serialize_extension<extensions::KHR_materials_iridescence>(type.extensions, type.khrMaterialIridescence);
        parsing::serialize_extension<extensions::KHR_materials_sheen>(type.extensions, type.khrMaterialSheen);
        parsing::serialize_extension<extensions::KHR_materials_specular>(type.extensions, type.khrMaterialSpecular);
        parsing::serialize_extension<extensions::KHR_materials_transmission>(type.extensions, type.khrMaterialTransmission);
        parsing::serialize_extension<extensions::KHR_materials_unlit>(type.extensions, type.khrMaterialUnlit);
        parsing::serialize_extension<extensions::KHR_materials_volume>(type.extensions, type.khrMaterialVolume);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Material& type)
    {
        parsing::parse_from("pbrMetallicRoughness", json, type.pbrMetallicRoughness);
        parsing::parse_from("normalTexture", json, type.normalTexture);
        parsing::parse_from("occlusionTexture", json, type.occlusionTexture);
        parsing::parse_from("emissiveTexture", json, type.emissiveTexture);
        parsing::parse_from("alphaMode", json, type.alphaMode);
        parsing::parse_from("alphaCutoff", json, type.alphaCutoff);
        parsing::parse_from("doubleSided", json, type.doubleSided);

        parsing::parse_extensions_and_extras(json, type);

        // Parse extensions
        for (auto& [name, ext] : type.extensions)
        {
            parsing::parse_extension<extensions::EXT_materials_displacement>(name, ext, type.extMaterialDisplacement);
            parsing::parse_extension<extensions::KHR_materials_anisotropy>(name, ext, type.khrMaterialAnisotropy);
            parsing::parse_extension<extensions::KHR_materials_clearcoat>(name, ext, type.khrMaterialClearcoat);
            parsing::parse_extension<extensions::KHR_materials_emissive_strength>(name, ext, type.khrMaterialEmissiveStrength);
            parsing::parse_extension<extensions::KHR_materials_ior>(name, ext, type.khrMaterialIor);
            parsing::parse_extension<extensions::KHR_materials_iridescence>(name, ext, type.khrMaterialIridescence);
            parsing::parse_extension<extensions::KHR_materials_sheen>(name, ext, type.khrMaterialSheen);
            parsing::parse_extension<extensions::KHR_materials_specular>(name, ext, type.khrMaterialSpecular);
            parsing::parse_extension<extensions::KHR_materials_transmission>(name, ext, type.khrMaterialTransmission);
            parsing::parse_extension<extensions::KHR_materials_unlit>(name, ext, type.khrMaterialUnlit);
            parsing::parse_extension<extensions::KHR_materials_volume>(name, ext, type.khrMaterialVolume);
        }
    }

    // ----------------- Primitive -----------------
    void to_json(nlohmann::json& json, const Primitive& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("attributes", json, type.attributes, true);
        parsing::serialize_to("targets", json, type.targets, !type.targets.empty());
        parsing::serialize_to("material", json, type.material, type.material > base::constants::not_set);
        parsing::serialize_to("indices", json, type.indices, type.indices > base::constants::not_set);
        parsing::serialize_to("mode", json, type.mode, type.mode > base::constants::not_set);

        // Serialize extensions
        parsing::serialize_extension<extensions::EXT_lights_shadows>(type.extensions, type.extLightShadows);
        parsing::serialize_extension<extensions::KHR_draco_mesh_compression>(type.extensions, type.khrDracoMeshCompression);
        parsing::serialize_extension<extensions::KHR_materials_variants>(type.extensions, type.khrMaterialVariants);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Primitive& type)
    {
        parsing::parse_from("attributes", json, type.attributes, true);
        parsing::parse_from("targets", json, type.targets);
        parsing::parse_from("material", json, type.material);
        parsing::parse_from("indices", json, type.indices);
        parsing::parse_from("mode", json, type.mode);

        parsing::parse_extensions_and_extras(json, type);

        // Parse extensions
        for (auto& [name, ext] : type.extensions)
        {
            parsing::parse_extension<extensions::EXT_lights_shadows>(name, ext, type.extLightShadows);
            parsing::parse_extension<extensions::KHR_draco_mesh_compression>(name, ext, type.khrDracoMeshCompression);
            parsing::parse_extension<extensions::KHR_materials_variants>(name, ext, type.khrMaterialVariants);
        }
    }

    // ----------------- Mesh -----------------
    void to_json(nlohmann::json& json, const Mesh& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("primitives", json, type.primitives, true);
        parsing::serialize_to("weights", json, type.weights, !type.weights.empty());

        parsing::serialize_extension<extensions::EXT_lights_shadows>(type.extensions, type.extLightShadows);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Mesh& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("primitives", json, type.primitives, true);
        parsing::parse_from("weights", json, type.weights);

        parsing::parse_extensions_and_extras(json, type);

        for (auto& [name, ext] : type.extensions)
            parsing::parse_extension<extensions::EXT_lights_shadows>(name, ext, type.extLightShadows);
    }

    // ----------------- Node -----------------
    void to_json(nlohmann::json& json, const Node& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("camera", json, type.camera, type.camera >= 0);
        parsing::serialize_to("skin", json, type.skin, type.skin >= 0);
        parsing::serialize_to("mesh", json, type.mesh, type.mesh >= 0);
        parsing::serialize_to("children", json, type.children, !type.children.empty());
        parsing::serialize_to("translation", json, type.translation, !compare_dvec3(type.translation, base::constants::dvec3zeros));
        parsing::serialize_to("rotation", json, type.rotation, !compare_dvec4(type.rotation, base::constants::dvec4quat_default));
        parsing::serialize_to("scale", json, type.scale, !compare_dvec3(type.scale, base::constants::dvec3ones));
        parsing::serialize_to("matrix", json, type.matrix, !compare_dmat4(type.matrix, base::constants::dmat4identity));
        parsing::serialize_to("weights", json, type.weights, !type.weights.empty());

        // Serialize extensions
        parsing::serialize_extension<extensions::KHR_audio>(type.extensions, type.khrAudio);
        parsing::serialize_extension<extensions::KHR_lights_punctual>(type.extensions, type.khrLightsPunctual);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Node& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("camera", json, type.camera);
        parsing::parse_from("skin", json, type.skin);
        parsing::parse_from("mesh", json, type.mesh);
        parsing::parse_from("children", json, type.children);
        parsing::parse_from("translation", json, type.translation);
        parsing::parse_from("rotation", json, type.rotation);
        parsing::parse_from("scale", json, type.scale);
        parsing::parse_from("matrix", json, type.matrix);
        parsing::parse_from("weights", json, type.weights);

        parsing::parse_extensions_and_extras(json, type);

        // Parse extensions
        for (auto& [name, ext] : type.extensions)
        {
            parsing::parse_extension<extensions::KHR_audio>(name, ext, type.khrAudio);
            parsing::parse_extension<extensions::KHR_lights_punctual>(name, ext, type.khrLightsPunctual);
        }
    }

    // ----------------- Sampler -----------------
    void to_json(nlohmann::json& json, const Sampler& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("minFilter", json, type.minFilter, type.minFilter > base::constants::not_set);
        parsing::serialize_to("magFilter", json, type.magFilter, type.magFilter > base::constants::not_set);
        parsing::serialize_to("wrapS", json, type.wrapS, type.wrapS != base::enumerators::texture_wrap::repeat);
        parsing::serialize_to("wrapT", json, type.wrapT, type.wrapT != base::enumerators::texture_wrap::repeat);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Sampler& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("minFilter", json, type.minFilter);
        parsing::parse_from("magFilter", json, type.magFilter);
        parsing::parse_from("wrapS", json, type.wrapS);
        parsing::parse_from("wrapT", json, type.wrapT);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Scene -----------------
    void to_json(nlohmann::json& json, const Scene& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("nodes", json, type.nodes, !type.nodes.empty());

        // Serialize extensions
        parsing::serialize_extension<extensions::KHR_audio>(type.extensions, type.khrAudio);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Scene& type)
    {
        // Base
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("nodes", json, type.nodes);

        parsing::parse_extensions_and_extras(json, type);

        // Parse extensions
        for (auto& [name, ext] : type.extensions)
        {
            parsing::parse_extension<extensions::KHR_audio>(name, ext, type.khrAudio);
        }
    }

    // ----------------- Skin -----------------
    void to_json(nlohmann::json& json, const Skin& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("inverseBindMatrices", json, type.inverseBindMatrices, type.inverseBindMatrices > base::constants::not_set);
        parsing::serialize_to("skeleton", json, type.skeleton, type.skeleton > base::constants::not_set);
        parsing::serialize_to("joints", json, type.joints, true);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Skin& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("inverseBindMatrices", json, type.inverseBindMatrices);
        parsing::parse_from("skeleton", json, type.skeleton);
        parsing::parse_from("joints", json, type.joints, true);

        parsing::parse_extensions_and_extras(json, type);
    }

    // ----------------- Texture -----------------
    void to_json(nlohmann::json& json, const Texture& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("name", json, type.name, !type.name.empty());
        parsing::serialize_to("sampler", json, type.sampler, type.sampler > base::constants::not_set);
        parsing::serialize_to("source", json, type.source, type.source > base::constants::not_set);

        // Serialize extensions
        parsing::serialize_extension<extensions::KHR_texture_basisu>(type.extensions, type.khrTextureBasisU);

        parsing::serialize_extensions_and_extras(json, type);
    }

    void from_json(const nlohmann::json& json, Texture& type)
    {
        parsing::parse_from("name", json, type.name);
        parsing::parse_from("sampler", json, type.sampler);
        parsing::parse_from("source", json, type.source);

        parsing::parse_extensions_and_extras(json, type);

        // Parse extensions
        for (auto& [name, ext] : type.extensions)
        {
            parsing::parse_extension<extensions::KHR_texture_basisu>(name, ext, type.khrTextureBasisU);
        }
    }

    // ----------------- GLTF -----------------
    void to_json(nlohmann::json& json, const FGLTFRoot& type)
    {
        json = nlohmann::json::object();

        parsing::serialize_to("asset", json, type.asset, type.asset != Asset{});

        parsing::serialize_to("accessors", json, type.accessors, !type.accessors.empty());
        parsing::serialize_to("animations", json, type.animations, !type.animations.empty());
        parsing::serialize_to("buffers", json, type.buffers, !type.buffers.empty());
        parsing::serialize_to("bufferViews", json, type.bufferViews, !type.bufferViews.empty());
        parsing::serialize_to("cameras", json, type.cameras, !type.cameras.empty());
        parsing::serialize_to("images", json, type.images, !type.images.empty());
        parsing::serialize_to("materials", json, type.materials, !type.materials.empty());
        parsing::serialize_to("meshes", json, type.meshes, !type.meshes.empty());
        parsing::serialize_to("nodes", json, type.nodes, !type.nodes.empty());
        parsing::serialize_to("samplers", json, type.samplers, !type.samplers.empty());
        parsing::serialize_to("scenes", json, type.scenes, !type.scenes.empty());
        parsing::serialize_to("skins", json, type.skins, !type.skins.empty());
        parsing::serialize_to("textures", json, type.textures, !type.textures.empty());

        // Serialize extensions
        parsing::serialize_extension<extensions::KHR_audio>(type.extensions, type.khrAudio);
        parsing::serialize_extension<extensions::KHR_lights_punctual>(type.extensions, type.khrLightsPunctual);
        parsing::serialize_extension<extensions::KHR_materials_variants>(type.extensions, type.khrMaterialVariants);

        parsing::serialize_extensions_and_extras(json, type);

        // Update used extensions
        type.extensionsUsed.clear();
        parsing::serialize_used_extensions(type.extensionsUsed, json);

        parsing::serialize_to("extensionsUsed", json, type.extensionsUsed, !type.extensionsUsed.empty());
        parsing::serialize_to("extensionsRequired", json, type.extensionsRequired, !type.extensionsRequired.empty());
    }

    void from_json(const nlohmann::json& json, FGLTFRoot& type)
    {
        parsing::parse_from("asset", json, type.asset, true);

        parsing::parse_from("accessors", json, type.accessors);
        parsing::parse_from("animations", json, type.animations);
        parsing::parse_from("buffers", json, type.buffers);
        parsing::parse_from("bufferViews", json, type.bufferViews);
        parsing::parse_from("cameras", json, type.cameras);
        parsing::parse_from("images", json, type.images);
        parsing::parse_from("materials", json, type.materials);
        parsing::parse_from("meshes", json, type.meshes);
        parsing::parse_from("nodes", json, type.nodes);
        parsing::parse_from("samplers", json, type.samplers);
        parsing::parse_from("scenes", json, type.scenes);
        parsing::parse_from("skins", json, type.skins);
        parsing::parse_from("textures", json, type.textures);

        parsing::parse_extensions_and_extras(json, type);

        // Parse extensions
        for (auto& [name, ext] : type.extensions)
        {
            parsing::parse_extension<extensions::KHR_audio>(name, ext, type.khrAudio);
            parsing::parse_extension<extensions::KHR_lights_punctual>(name, ext, type.khrLightsPunctual);
            parsing::parse_extension<extensions::KHR_materials_variants>(name, ext, type.khrMaterialVariants);
        }

        parsing::parse_from("extensionsUsed", json, type.extensionsUsed);
        parsing::parse_from("extensionsRequired", json, type.extensionsRequired);
    }
}