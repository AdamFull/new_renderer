#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

namespace engine
{
    namespace world
    {
        class CGameObject
        {
        public:
            template<typename _Ty, typename... _Args>
            _Ty& AddComponent(_Args&&... args)
            {
                return m_pRegistry->emplace<_Ty>(m_self, std::forward<_Args>(args)...);
            }

            template<typename _Ty>
            _Ty* GetComponent()
            {
                return m_pRegistry->try_get<_Ty>(m_self);
            }

            template<typename _Ty>
            void RemoveComponent()
            {
                m_pRegistry->remove<_Ty>(m_self);
            }

            template<typename _Ty>
            bool HasComponent()
            {
                return m_pRegistry->any_of<_Ty>(m_self);
            }

            template<typename _Ty>
            _Ty* GetComponentInChildren()
            {

            }

            template<typename _Ty>
            _Ty* GetComponentInParent()
            {

            }

            void AddChild(CGameObject* child);
            void RemoveChild(CGameObject* child);
            CGameObject* GetParent();

            entt::entity GetEntity() const { return m_self; }

        private:
            void SetParent(CGameObject* parent)
            {

            }
        private:
            entt::entity m_self;
            entt::registry* m_pRegistry{ nullptr };
        };
    }
}