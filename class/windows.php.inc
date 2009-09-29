<?php
// - - - - - - - - - - - - - BEGIN LICENSE BLOCK - - - - - - - - - - - - -
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is sitefusion.sourceforge.net code.
//
// The Initial Developer of the Original Code is
// FrontDoor Media Group.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -


/**
 * @package API
 * @subpackage Windows
*/



/**
 * Basic window functionality
 * 
 * Not to be constructed directly
*/

class XULBasicWindow extends BranchNode
{
	public function preventClose( $bool = TRUE ) {
		$this->setProperty( 'windowObject.preventClose', (bool) $bool );
	}

	public function size( $width, $height ) {
		$this->callMethod( 'windowObject.resizeTo', array($width,$height) );
	}

	public function alert( $mesg ) {
		$this->callMethod( 'alert', array($mesg) );
	}
	
	public function blur() {
		$this->callMethod( 'windowObject.blur' );
	}

	public function focus() {
		$this->callMethod( 'windowObject.focus' );
	}
	
	public function sizeToContent() {
		$this->callMethod( 'sizeToContent' );
	}
	
	public function maximize() {
		$this->callMethod( 'maximize' );
	}
	
	public function minimize() {
		$this->callMethod( 'minimize' );
	}
	
	public function addStyleSheetRule() {
		$rules = func_get_args();
		
		for( $n = 0; $n < count($rules); $n++ ) {
			if( is_array($rules[$n]) ) {
				array_splice( $rules, $n, 1, $rules[$n] );
				$n--;
				continue;
			}
			elseif( is_string($rules[$n]) )
				$this->callMethod( 'addStyleSheetRule', array( $rules[$n] ) );
		}
	}

	public function openLink( $url ) {
		$this->callMethod( 'openLink', array($url) );
	}
}


/**
 * Window element
 * 
 * This is the root window class. It is constructed automatically once upon application launch,
 * and should not be constructed separately. Use XULChildWindow or XULDialog for additional 
 * application windows.
 * 
 * @see XULChildWindow
 * @see XULDialog
 * @link https://developer.mozilla.org/en/XUL/window
*/

class XULWindow extends XULBasicWindow
{
	public $hasClosed		= FALSE;
	public $application;
	public $operatingSystem = '';
	public $workingDirectory = '';
	
	public function __construct() {
		$this->hostWindow = $this;
	}
	
	public function attach() {
		$this->parent = NULL;
	}
	
	public function open() {
		$this->callMethod( 'windowObject.focus' );
	}
	
	public function close() {
		$this->application->onWindowClose();
		
		$this->callMethod( 'windowObject.close' );
	}
	
	public function title( $title ) {
		$this->callMethod( 'setTitle', array($title) );
	}
	
	public function setStatusBarComponent( $obj ) {
		if(! $obj instanceof StatusBarComponent )
			Error::CallerFatal( 'Object is not a StatusBarComponent' );
		
		$this->statusBarComponent = $obj;
		
		if( $this->isRegistered )
			$this->setProperty( 'statusBarComponent', $obj );
	}
}


/**
 * Child window element
 * 
 * @link https://developer.mozilla.org/en/XUL/window
*/

class XULChildWindow extends XULBasicWindow
{
	public $alwaysLowered	= FALSE;
	public $alwaysRaised	= FALSE;
	public $modal		= FALSE;
	public $centerscreen	= TRUE;
	public $dependent	= TRUE;
	public $dialog		= FALSE;
	public $resizable	= TRUE;
	public $title		= 'SiteFusion ChildWindow';
	public $hasClosed	= FALSE;
	public $remoteConstructor = 'ChildWindow';
	
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->title( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->buttons = array_shift($args);
			if( count($args) && is_bool($args[0]) )
				$this->centerscreen = array_shift($args);
			if( count($args) && is_bool($args[0]) )
				$this->dependent = array_shift($args);
			
			if( count($args) )
				$this->initChildren = $args;
		}
	}
	
	public function attach() {
		$this->hasClosed = FALSE;
		$this->hostWindow = $this;

		$this->createRemoteObject();
		$this->setProperty( 'alwaysLowered', $this->alwaysLowered );
		$this->setProperty( 'alwaysRaised', $this->alwaysRaised );
		$this->setProperty( 'modal', $this->modal );
		$this->setProperty( 'centerscreen', $this->centerscreen );
		$this->setProperty( 'dependent', $this->dependent );
		$this->setProperty( 'dialog', $this->dialog );
		$this->setProperty( 'resizable', $this->resizable );
		$this->setProperty( 'hostWindow', $this->hostWindow );
		$this->setProperty( 'parentHostWindow', $this->parent->hostWindow );
		$this->callMethod( 'open' );
		
		$this->rootApplication->sendCommand( "R[".$this->parent->id."].addChildSilent(R[".$this->id."]);" );
		
		$this->setEventHandler( 'initialized', $this, 'initWindow' );
	}
	
	public function detach() {
		$this->rootApplication->sendCommand( "R[".$this->parent->id."].removeChildSilent(R[".$this->id."]);" );
		$this->unRegister();
	}
	
	public function close() {
		$this->callMethod( 'close' );
		$this->closeWindow();
	}
	
	public function initWindow() {
		$this->title( $this->title );
		$this->setEventHandler( 'hasClosed', $this, 'closeWindow' );
		$this->focus();
		$this->init();
	}

	public function init() {
		if( isset($this->initChildren) )
			$this->addChild( $this->initChildren );
		
		$this->sizeToContent();
		
		if( $this->dependent && $this->centerscreen )
			$this->callMethod( 'windowObject.moveToAlertPosition' );
		elseif( $this->centerscreen )
			$this->callMethod( 'windowObject.centerWindowOnScreen' );
	}

	public function closeWindow() {
		$this->hasClosed = TRUE;
		$this->parent->removeChild( $this );
	}
	
	public function title( $title = NULL ) {
		if( $title === NULL )
			return $this->title;
		
		$this->title = $title;
		
		if( $this->isRegistered )
			$this->callMethod( 'setTitle', array($title) );
		
		return $this;
	}
}


/**
 * Dialog element
 * 
 * @link https://developer.mozilla.org/en/XUL/dialog
*/

class XULDialog extends XULChildWindow
{
	public $alwaysLowered	= FALSE;
	public $alwaysRaised	= FALSE;
	public $modal		= FALSE;
	public $centerscreen	= TRUE;
	public $dependent	= TRUE;
	public $dialog		= TRUE;
	public $resizable	= FALSE;
	public $title		= 'SiteFusion Dialog';

	public $buttons		= 'accept,cancel';
	public $remoteConstructor = 'Dialog';
	public $initAttributes = array( 'buttonlabelaccept', 'buttonlabelcancel', 'buttonlabelhelp', 'buttonlabeldisclosure' );
	
	
	public function initWindow() {
		$this->title( $this->title );
		$this->setProperty( 'element.buttons', $this->buttons );
		$this->setEventHandler( 'hasClosed', $this, 'closeWindow' );
		$this->init();
	}
	
	public function buttonlabelaccept( $label = NULL ) {
		if( $label === NULL )
			return $this->buttonlabelaccept;
		
		$this->buttonlabelaccept = $label;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.getButton("accept").label', $label );
		
		return $this;
	}

	public function buttonlabelcancel( $label = NULL ) {
		if( $label === NULL )
			return $this->buttonlabelcancel;
		
		$this->buttonlabelcancel = $label;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.getButton("cancel").label', $label );
		
		return $this;
	}

	public function buttonlabelhelp( $label = NULL ) {
		if( $label === NULL )
			return $this->buttonlabelhelp;
		
		$this->buttonlabelhelp = $label;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.getButton("help").label', $label );
		
		return $this;
	}

	public function buttonlabeldisclosure( $label = NULL ) {
		if( $label === NULL )
			return $this->buttonlabeldisclosure;
		
		$this->buttonlabeldisclosure = $label;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.getButton("disclosure").label', $label );
		
		return $this;
	}
}


/**
 * OS-native prompt service object
 *
 * @property bool	$result		Whether the dialog was accepted by the user (e.g. clicked OK)
 * @property bool	$checkState	State of the checkbox (if any)
 * @property mixed	$value		Value of the textbox or selected item in the list
 * @property string	$username	Supplied username
 * @property string	$password	Supplied password
 */

class PromptService extends Node
{
	public $remoteConstructor = 'PromptService';
	
	public $promptInProgress = FALSE;
	private $handlerObj = NULL;
	private $handlerMethod = NULL;
	private $carryValue = NULL;
	
	public function attach() {
		parent::attach();
		$this->setEvent( 'yield', MSG_SEND, $this, 'yieldCollect' );
	}
	
	
	/**
	 * Shows a simple alert box with an OK button
	 *
	 * @param string	$title	Title of the dialog box
	 * @param string	$text	Body text
	 */
	
	public function alert( $title, $text ) {
		$this->callMethod( 'alert', array($title,$text) );
	}
	
	
	/**
	 * Shows a simple alert box with an OK button and a checkbox
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function alertCheck( $title, $text, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'alertCheck', array($title,$text,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a simple confirm box with OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function confirm( $title, $text, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'confirm', array($title,$text) );
	}
	
	/**
	 * Shows a simple confirm box with OK and Cancel buttons and a checkbox
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function confirmCheck( $title, $text, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'confirmCheck', array($title,$text,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a simple prompt box with a textinput and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$textValue		Initial value of the textinput
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function prompt( $title, $text, $textValue, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'prompt', array($title,$text,$textValue,NULL,NULL) );
	}
	
	
	/**
	 * Shows a simple prompt box with a textinput, a checkbox and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$textValue		Initial value of the textinput
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptCheck( $title, $text, $textValue, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'prompt', array($title,$text,$textValue,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a simple prompt box with username and password textinputs and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$username		Initial value of the username textinput
	 * @param string	$password		Initial value of the password textinput
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptUsernameAndPassword( $title, $text, $username, $password, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'promptUsernameAndPassword', array($title,$text,$username,$password,NULL,NULL) );
	}
	
	
	/**
	 * Shows a simple prompt box with username and password textinputs, a checkbox and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$username		Initial value of the username textinput
	 * @param string	$password		Initial value of the password textinput
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptUsernameAndPasswordCheck( $title, $text, $username, $password, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'promptUsernameAndPassword', array($title,$text,$username,$password,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a simple prompt box with a password textinput and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$password		Initial value of the password textinput
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptPassword( $title, $text, $password, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'promptPassword', array($title,$text,$password,NULL,NULL) );
	}
	
	
	/**
	 * Shows a simple prompt box with a password textinput, a checkbox and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$password		Initial value of the password textinput
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptPasswordCheck( $title, $text, $password, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'promptPassword', array($title,$text,$password,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a simple prompt box with a dropdown box and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param array		$list			Array of options (strings)
	 * @param int		$selectedIndex	Initial index to select (broken)
	 * @param object	$handlerObj		Object to call the callback function on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function select( $title, $text, $list, $selectedIndex, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) Error::CallerFatal( 'PromptService: a prompt is already in progress' );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'select', array($title,$text,$list,(int)$selectedIndex) );
	}
	
	
	/**
	 * [INTERNAL FUNCTION] Collects the results of a prompt
	*/
	
	public function yieldCollect( $e, $result, $checkState, $value, $username, $password ) {
		$this->result = $result;
		$this->checkState = $checkState;
		$this->value = $value;
		$this->username = $username;
		$this->password = $password;
		
		$this->promptInProgress = FALSE;
		$ho = $this->handlerObj;
		$hm = $this->handlerMethod;
		$cv = $this->carryValue;
		$this->handlerObj = NULL;
		$this->handlerMethod = NULL;
		$this->carryValue = NULL;
		
		$ho->{$hm}( $this, $cv );
	}
}

