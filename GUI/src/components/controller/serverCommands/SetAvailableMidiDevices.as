/* Integra Live graphical user interface
 *
 * Copyright (C) 2009 Birmingham City University
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA   02110-1301,
 * USA.
 */


package components.controller.serverCommands
{
	import components.controller.ServerCommand;
	import components.model.IntegraModel;
	import components.model.preferences.MidiSettings;
	
	import flexunit.framework.Assert;
	

	/* 
	note - this is an unusual ServerCommand, in that it is only ever used as a 'remote command' - ie
	dispatched by RemoteCommandHandler.  It is never dispatched by the user, via a view.  Therefore,
	some of it's methods (generateInverse / executeServerCommand / testServerResponse ) are not needed
	*/
	
	public class SetAvailableMidiDevices extends ServerCommand
	{
		public function SetAvailableMidiDevices( availableInputDevices:Vector.<String>, availableOutputDevices:Vector.<String> )
		{
			super();

			_availableInputDevices = availableInputDevices;
			_availableOutputDevices = availableOutputDevices;
		}
		
		
		public function get availableInputDevices():Vector.<String> { return _availableInputDevices; }
		public function get availableOutputDevices():Vector.<String> { return _availableOutputDevices; }
	
		
		public override function initialize( model:IntegraModel ):Boolean
		{
			return true;
        }
	
		
		public override function generateInverse( model:IntegraModel ):void
		{
			//not needed - see note above				
		}
		
		
		public override function execute( model:IntegraModel ):void
		{
			var midiSettings:MidiSettings = model.midiSettings;
			Assert.assertNotNull( midiSettings );
			
			if( _availableInputDevices ) midiSettings.availableInputDevices = _availableInputDevices;
			if( _availableOutputDevices ) midiSettings.availableOutputDevices = _availableOutputDevices;
			
			midiSettings.hasChangedSinceReset = true;
		}
		
		
		public override function executeServerCommand( model:IntegraModel ):void
		{
			//not needed - see note above				
		}
		
		
		override protected function testServerResponse( response:Object ):Boolean
		{
			//not needed - see note above				
        	return true;
		}

		
		public override function getAttributesChangedByThisCommand( model:IntegraModel, changedAttributes:Vector.<String> ):void
		{
			changedAttributes.push( model.getPathStringFromID( model.midiSettings.id ) + ".availableInputDevices" );
			changedAttributes.push( model.getPathStringFromID( model.midiSettings.id ) + ".availableOutputDevices" );
		}	
		

		private var _availableInputDevices:Vector.<String>;
		private var _availableOutputDevices:Vector.<String>;		
	}
}
