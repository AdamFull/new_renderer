#pragma once

#include <container/flags.h>
#include <container/hash_map.h>

namespace engine
{
	enum class ERenderVariableFlagBits
	{
		eMutable = 1 << 0,
		eVisible = 1 << 1
	};
	UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ERenderVariableFlags, ERenderVariableFlagBits)

	class IVariable
	{
	public:
		virtual ~IVariable() = default;

		virtual void display() = 0;

		void setFlag(ERenderVariableFlagBits flag) { m_flags |= flag; }
		void resetFlag(ERenderVariableFlagBits flag) { m_flags &= ~flag; }

		const ERenderVariableFlags& getFlags() const { return m_flags; }

	protected:
		ERenderVariableFlags m_flags{};
		std::string m_srDisplayName{};
	};

	class IVariableBlock
	{
	public:
		virtual ~IVariableBlock() = default;

		virtual void display() = 0;

		void addVariable(const char* name, std::unique_ptr<IVariable>&& var)
		{
			// TODO: check for collisions
			m_variables.emplace(utl::fnv1a_32_hash_cstr(name), std::move(var));
		}

		void addBlock(const char* name, std::unique_ptr<IVariableBlock>&& block)
		{
			// TODO: check for collisions
			m_blocks.emplace(utl::fnv1a_32_hash_cstr(name), std::move(block));
		}

	protected:
		utl::hash_map<u64, std::unique_ptr<IVariable>> m_variables{};
		utl::hash_map<u64, std::unique_ptr<IVariableBlock>> m_blocks{};
	};
}