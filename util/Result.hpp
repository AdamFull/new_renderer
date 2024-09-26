#pragma once

#include <variant>
#include <functional>
#include <stdexcept>

namespace engine
{
	template<typename _Ty, typename _ErrTy>
	class Result
	{
	public:
		static Result Success(_Ty&& val)
		{
			return Result(std::move(val));
		}

		static Result Failure(_ErrTy err)
		{
			return Result(err);
		}

		bool IsSuccess() const 
		{
			return std::holds_alternative<_Ty>(result);
		}

		const _Ty& Value() const 
		{
			if (!IsSuccess()) 
			{
				throw std::logic_error("No value present, it's an error result.");
			}

			return std::get<_Ty>(result);
		}

		const _ErrTy& Error() const 
		{
			if (IsSuccess()) 
			{
				throw std::logic_error("No error present, it's a success result.");
			}

			return std::get<_ErrTy>(result);
		}

		Result& OnError(const std::function<void(const _ErrTy&)>& onError) const
		{
			if (onError && !IsSuccess())
			{
				onError(std::get<_ErrTy>(result));
			}

			return *this;
		}

		Result& OnSuccess(const std::function<void(const _Ty&)>& onSuccess) const
		{
			if (onSuccess && IsSuccess())
			{
				onSuccess(std::get<_Ty>(result));
			}

			return *this;
		}

	private:
		// Private constructor to prevent invalid usage
		Result(_Ty&& value) : result(std::move(value)) {}
		Result(_ErrTy error) : result(error) {}

		std::variant<_Ty, _ErrTy> result;
	};
}