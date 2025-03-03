#pragma once

#include <string>
#include <memory>

namespace edge
{
	namespace filesystem
	{
        class IDirectoryIteratorInterface;

        /**
        * @class IDirectoryIterator
        * @brief This class is an interface for iterating over directories.
        */
        class IDirectoryIterator
        {
        public:
            /**
             * @brief Virtual destructor.
             */
            virtual ~IDirectoryIterator() = default;

            /**
             * @brief Move to the next item in the directory.
             */
            virtual void _OperatorIncrement() = 0;

            /**
             * @brief Get the current item in the directory.
             *
             * @return A constant reference to the item's string representation.
             */
            virtual auto _OperatorDereferencing() const noexcept -> std::string const& = 0;

            /**
             * @brief Compare the directory iterators for inequality.
             *
             * @param other A pointer to the IDirectoryIteratorInterface to compare.
             * @return True if the iterators are not equal, false otherwise.
             */
            virtual auto _OperatorNotEqual(const IDirectoryIteratorInterface*) const noexcept -> bool = 0;
        };


		class IDirectoryIteratorInterface
		{
		public:
			virtual ~IDirectoryIteratorInterface() = default;

            /**
            * @brief Get the iterator pointing to the beginning of the directory.
            *
            * @return A reference to the iterator pointing to the beginning of the directory.
            */
            [[nodiscard]] virtual auto Begin() -> IDirectoryIteratorInterface & = 0;

            /**
             * @brief Get the iterator pointing to the end of the directory.
             *
             * @return A reference to the iterator pointing to the end of the directory.
             */
            [[nodiscard]] virtual auto End() -> IDirectoryIteratorInterface & = 0;

            /**
             * @brief Move to the next item in the directory.
             *
             * @return A reference to the updated iterator.
             */
            [[nodiscard]] virtual auto operator++() ->IDirectoryIteratorInterface & = 0;

            /**
             * @brief Get the current item in the directory.
             *
             * @return A constant reference to the item's string representation.
             */
            [[nodiscard]] virtual auto operator*() -> std::string const& = 0;

            /**
             * @brief Compare the directory iterators for inequality.
             *
             * @param other A reference to the DirectoryIterator to compare.
             * @return True if the iterators are not equal, false otherwise.
             */
            [[nodiscard]] virtual auto operator!=(const IDirectoryIteratorInterface& other) const -> bool = 0;

            /**
             * @brief Unique_ptr to an IDirectoryIterator object.
             */
            std::unique_ptr<IDirectoryIterator> it_{ nullptr };
		};
	}
}