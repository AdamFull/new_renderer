#pragma once

#include <container/flags.h>
#include <container/hash_map.h>

namespace engine
{
	namespace render
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
			std::string& m_srDisplayName{};
		};

		class IVariableGroup
		{
		public:
			virtual ~IVariableGroup() = default;

			virtual void display() = 0;

			// addVariable (name, variable&&)
			// removeVariable(name)

			// getBlockMemory()

		protected:
			utl::hash_map<u64, u64> m_variableRemap{};
			utl::vector<std::unique_ptr<IVariable>> m_vVariables{};
		};
	}
}