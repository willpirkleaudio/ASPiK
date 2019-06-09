// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iappdelegate.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Application {

enum class ConfigKey : uint64_t;
struct ConfigValue;

//------------------------------------------------------------------------
/** Startup configuration
 *
 *	The standalone library can be configured with a list of key-value pairs.
 *	See ConfigKey for a list and description of available keys.
 *
 *	@ingroup standalone
 */
using Configuration = std::vector<std::pair<ConfigKey, ConfigValue>>;

//------------------------------------------------------------------------
/** Configuration keys
 *
 *	Enumeration of available configuration keys.
 *
 *	@ingroup standalone
 */
enum class ConfigKey : uint64_t
{
	/** Instead of plain xml files, use compressed ui description files.
	 *
	 *	This option expects an integer ConfigValue where 0 means that plain xml files are used and
	 *	any other value means that the ui description file is compressed. In this case for
	 *	development purposes an uncompressed xml file is also written.
	 */
	UseCompressedUIDescriptionFiles,
};

//------------------------------------------------------------------------
/** Configuration Value
 *
 *	@ingroup standalone
 */
struct ConfigValue
{
	ConfigValue () = delete;
	ConfigValue (int64_t v) : type (Type::Integer) { value.integer = v; }
	ConfigValue (const char* s) : type (Type::String) { value.string = s; }

	enum class Type
	{
		Unknown,
		Integer,
		String
	} type = Type::Unknown;

	union
	{
		int64_t integer;
		const char* string;
	} value = {};
};

//------------------------------------------------------------------------
/** Init application
 *
 *	@see IDelegate
 *
 *	@ingroup standalone
 */
struct Init
{
	explicit Init (DelegatePtr&& delegate, Configuration&& config = {});
};

//------------------------------------------------------------------------
} // Application
} // Standalone
} // VSTGUI
