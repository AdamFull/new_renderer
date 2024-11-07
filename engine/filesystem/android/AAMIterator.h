#pragma once

#include "DirectoryIterator.h"
#include <android/asset_manager.h>

namespace engine
{
    namespace filesystem
    {
        /**
         * @class AAMDirectoryIterator
         * @brief This class provides an iterator for android internal storage.
         *
         * AAMDirectoryIterator is a derived class from IDirectoryIterator and implements its
         * interface for iterating through the files within a android internal storage.
         */
        class AAMDirectoryIterator final : public IDirectoryIterator
        {
        public:
            friend class CAAMRecursiveDirectoryIterator;

            /**
             * @brief Default constructor.
             */
            AAMDirectoryIterator() = default;

            /**
             * @brief Construct a AAMDirectoryIterator with a asset manager and a starting path.
             *
             * @param handle A pointer to the android asset manager.
             * @param start_path A string representing the starting path within the archive.
             */
            AAMDirectoryIterator(AAssetManager* handle, const std::string& start_path);

            /**
             * @brief Virtual destructor.
             */
            virtual ~AAMDirectoryIterator() override;
        protected:
            /**
             * @brief Move to the next item in the archive.
             */
            void _OperatorIncrement() override;

            /**
             * @brief Get the current item in the archive.
             *
             * @return A constant reference to the item's string representation.
             */
            const std::string& _OperatorDereferencing() const override;

            /**
             * @brief Compare the directory iterators for inequality.
             *
             * @param other A pointer to the DirectoryIterator to compare.
             * @return True if the iterators are not equal, false otherwise.
             */
            bool _OperatorNotEqual(const DirectoryIterator*) const override;
        private:
            std::string m_srCurrentPath{};
            AAssetManager* m_pAssetMgr{ nullptr };
            AAssetDir* m_pCurrentDir{ nullptr };
        };

        /**
         * @class CAAMRecursiveDirectoryIterator
         * @brief This class provides an iterator for android internal storage.
         *
         * CAAMRecursiveDirectoryIterator is a derived class from IDirectoryIterator and implements its
         * interface for iterating through the files within a android internal storage.
         */
        class CAAMRecursiveDirectoryIterator final : public IDirectoryIterator
        {
        public:
            /**
             * @brief Default constructor.
             */
            CAAMRecursiveDirectoryIterator() = default;

            /**
             * @brief Construct a CAAMRecursiveDirectoryIterator with a asset manager and a starting path.
             *
             * @param handle A pointer to the android asset manager.
             * @param start_path A string representing the starting path within the archive.
             */
            CAAMRecursiveDirectoryIterator(AAssetManager* handle, const std::string& start_path);

            /**
             * @brief Virtual destructor.
             */
            virtual ~CAAMRecursiveDirectoryIterator() override;
        protected:
            /**
             * @brief Move to the next item in the archive.
             */
            void _OperatorIncrement() override;

            /**
             * @brief Get the current item in the archive.
             *
             * @return A constant reference to the item's string representation.
             */
            const std::string& _OperatorDereferencing() const override;

            /**
             * @brief Compare the directory iterators for inequality.
             *
             * @param other A pointer to the DirectoryIterator to compare.
             * @return True if the iterators are not equal, false otherwise.
             */
            bool _OperatorNotEqual(const DirectoryIterator*) const override;
        private:
            AAssetManager* m_pAssetMgr{ nullptr };
            std::stack<AAMDirectoryIterator> m_iterStack;
            std::string m_srCurrentPath{};
        };
    }
}