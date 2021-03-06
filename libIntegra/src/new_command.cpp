/* libIntegra modular audio framework
 *
 * Copyright (C) 2007 Birmingham City University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
 * USA.
 */


#include "platform_specifics.h"

#include "new_command.h"
#include "server.h"
#include "node.h"
#include "module_manager.h"
#include "dsp_engine.h"
#include "api/trace.h"
#include "logic.h"
#include "api/string_helper.h"
#include "api/command_result.h"

#include <assert.h>


namespace integra_api
{
	INewCommand *INewCommand::create( const GUID &module_id, const string &node_name, const CPath &parent_path )
	{
		return new integra_internal::CNewCommand( module_id, node_name, parent_path );
	}
}


namespace integra_internal
{
	CNewCommand::CNewCommand( const GUID &module_id, const string &node_name, const CPath &parent_path )
	{
		m_module_id = module_id;
		m_node_name = node_name;
		m_parent_path = parent_path;
	}


	CError CNewCommand::execute( CServer &server, CCommandSource source, CCommandResult *result )
	{
		/* get interface */
		const CInterfaceDefinition *interface_definition = CInterfaceDefinition::downcast( server.find_interface( m_module_id ) );
		if( !interface_definition ) 
		{
			INTEGRA_TRACE_ERROR << "unable to find interface";
			return CError::FAILED;
		}

		/* if node name is NULL, create one */
		if( m_node_name.empty() )
		{
			m_node_name = make_node_name( server, interface_definition->get_interface_info().get_name() );
		}

		/* First check if node name is already taken */
		CNode *parent = server.find_node_writable( m_parent_path );
		node_map &sibling_map = parent ? parent->get_children_writable() : server.get_nodes_writable();
		while( sibling_map.count( m_node_name ) > 0 ) 
		{
			INTEGRA_TRACE_PROGRESS << "node name is in use; appending underscore" << m_node_name;

			m_node_name += "_";
		}

		if( !CStringHelper::validate_node_name( m_node_name ) )
		{
			INTEGRA_TRACE_ERROR << "node name contains invalid characters" << m_node_name;
			return CError::FAILED;
		}

		CNode *node = new CNode;
		node->initialize( *interface_definition, m_node_name, server.create_internal_id(), parent );

		if( !node->get_logic().can_be_child_of( parent ) )
		{
			INTEGRA_TRACE_ERROR << interface_definition->get_interface_info().get_name() << " cannot be created as child of " << parent ? parent->get_interface_definition().get_interface_info().get_name() : "top level";
			delete node;
			return CError::TYPE_ERROR;
		}

		sibling_map[ m_node_name ] = node;
		server.get_state_table().add( *node );

		if( interface_definition->has_implementation() )
		{
			/* load implementation in module host */
			CModuleManager &module_manager = CModuleManager::downcast( server.get_module_manager() );
			string patch_path = module_manager.get_patch_path( *interface_definition );
			if( patch_path.empty() )
			{
				INTEGRA_TRACE_ERROR << "Failed to get implementation path - cannot load module in host";
			}
			else
			{
				server.get_dsp_engine().add_module( node->get_id(), patch_path );
			}
		}

		/* set attribute defaults */
		const endpoint_definition_list &endpoint_definitions = interface_definition->get_endpoint_definitions();
		for( endpoint_definition_list::const_iterator i = endpoint_definitions.begin(); i != endpoint_definitions.end(); i++ )
		{
			const IEndpointDefinition *endpoint_definition = *i;
			if( endpoint_definition->get_type() != CEndpointDefinition::CONTROL || endpoint_definition->get_control_info()->get_type() != CControlInfo::STATEFUL )
			{
				continue;
			}

			const INodeEndpoint *node_endpoint = node->get_node_endpoint( endpoint_definition->get_name() );
			assert( node_endpoint );

			server.process_command( ISetCommand::create( node_endpoint->get_path(), endpoint_definition->get_control_info()->get_state_info()->get_default_value() ), CCommandSource::INITIALIZATION );
		}

		/* handle any system class logic */
		node->get_logic().handle_new( server, source );

		INTEGRA_TRACE_VERBOSE << "Created node: " << node->get_name();

		if( result )
		{
                        // TODO: this was to work around a bug on 10.7 and 10.8. re-add dynamic_cast when we stop supporting OS X 10.8 and earlier
			// CNewCommandResult *new_command_result = dynamic_cast<CNewCommandResult *> ( result );
			CNewCommandResult *new_command_result = (CNewCommandResult *)result;
			if( new_command_result )
			{
				new_command_result->set_created_node( node );
			}
			else
			{
				INTEGRA_TRACE_ERROR << "incorrect command result type - can't store result";
			}
		}

		return CError::SUCCESS;
	}


	string CNewCommand::make_node_name( CServer &server, const string &module_name ) const
	{
		ostringstream stream;
		stream << module_name << server.create_internal_id();
	
		return stream.str();
	}
}
