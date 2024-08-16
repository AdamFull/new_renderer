#pragma once

#include <container/flags.h>
#include <hash.hpp>

namespace engine
{
	enum class ERenderVariableFlagBits
	{
		eMutable = 1 << 0,
		eVisible = 1 << 1
	};
	UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ERenderVariableFlags, ERenderVariableFlagBits)

	enum class EVariableType
	{
		eScalar,
		eVector
	};

	enum EVariableDataType
	{
		eBool,
		eInt,
		eUInt,
		eInt64,
		eUInt64,
		eFloat,
		eDouble
	};

	template<class _Ty>
	struct FNumericScalarInitialData
	{
		_Ty value{};
		_Ty min{std::numeric_limits<_Ty>::min()};
		_Ty max{std::numeric_limits<_Ty>::max()};
	};

	template<class _Ty>
	struct FNumericVectorInitialData
	{
		utl::vector<_Ty> values{};
		_Ty min{ std::numeric_limits<_Ty>::min() };
		_Ty max{ std::numeric_limits<_Ty>::max() };
	};

	struct FVariableCreateInfo
	{
		EVariableType type{ EVariableType::eScalar };
		EVariableDataType dtype{};
		void* initial{};
	};

	class IVariable
	{
	public:
		virtual ~IVariable() = default;

		virtual void Display() = 0;

		void SetFlag(ERenderVariableFlagBits flag) { m_flags |= flag; }
		void ResetFlag(ERenderVariableFlagBits flag) { m_flags &= ~flag; }

		const ERenderVariableFlags& getFlags() const { return m_flags; }

	protected:
		ERenderVariableFlags m_flags{};
		std::string m_srDisplayName{};
	};

	class IVariableBlock
	{
	public:
		virtual ~IVariableBlock() = default;

		virtual void Display() = 0;

		void AddVariable(const char* name, std::unique_ptr<IVariable>&& var)
		{
			// TODO: check for collisions
			m_variables.emplace(utl::murmur_hash(name), std::move(var));
		}

		void AddBlock(const char* name, std::unique_ptr<IVariableBlock>&& block)
		{
			// TODO: check for collisions
			m_blocks.emplace(utl::murmur_hash(name), std::move(block));
		}

	protected:
		utl::hash_map<u64, std::unique_ptr<IVariable>> m_variables{};
		utl::hash_map<u64, std::unique_ptr<IVariableBlock>> m_blocks{};
	};

	class IVariableManager 
	{
	public:
		virtual ~IVariableManager() = default;

		virtual void Display() = 0;

		virtual std::unique_ptr<IVariable> MakeVariable(const char* name, const FVariableCreateInfo& create_info) = 0;
		virtual std::unique_ptr<IVariableBlock> MakeVariableBlock(const char* name) = 0;
	};
}