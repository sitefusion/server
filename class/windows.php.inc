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
		$this->setProperty( 'preventClose', (bool) $bool );
	}

	public function size( $width = NULL, $height = NULL ) {
		if( $width && $height )
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
	
	public function fullScreen( $state = TRUE ) {
		$this->callMethod( 'fullScreen', array((bool)$state) );
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
	
	public function openUrlWindow($url, $options="chrome")
	{
		$this->callMethod( 'openUrlWindow', array($url, $options) );
	}
	
	public function openUrlDialog($url, $options="chrome")
	{
		$this->callMethod( 'openUrlDialog', array($url, $options) );
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
	public $centerscreen	= FALSE;
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
		
		//$this->rootApplication->sendCommand( "R[".$this->parent->id."].addChildSilent(R[".$this->id."]);" );
		$this->parent->callMethod( 'addChildSilent', array($this) );
		
		$this->setEventHandler( 'initialized', $this, 'initWindow' );
	}
	
	public function detach() {
		//$this->rootApplication->sendCommand( "R[".$this->parent->id."].removeChildSilent(R[".$this->id."]);" );
		$this->parent->callMethod( 'removeChildSilent', array($this) );
		$this->unRegister();
	}
	
	public function close() {
		$this->callMethod( 'close' );
		$this->closeWindow();
	}
	
	public function initWindow( $event ) {
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
	public $initAttributes = array( 'buttonlabelaccept', 'buttonlabelcancel', 'buttonlabelhelp', 'buttonlabeldisclosure', 'buttonlabelextra1', 'buttonlabelextra2' );
	
	
	public function initWindow( $event ) {
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
	
	public function buttonlabelextra1( $label = NULL ) {
		if( $label === NULL )
			return $this->buttonlabelextra1;
		
		$this->buttonlabelextra1 = $label;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.getButton("extra1").label', $label );
		
		return $this;
	}
	
	public function buttonlabelextra2( $label = NULL ) {
		if( $label === NULL )
			return $this->buttonlabelextra2;
		
		$this->buttonlabelextra2 = $label;
		
		if( $this->isRegistered )
			$this->setProperty( 'element.getButton("extra2").label', $label );
		
		return $this;
	}
	
	public function disableButton($button = "accept", $disabled = TRUE) {
		if ($disabled)
			$this->callMethod("element.getButton('".$button."').setAttribute", "disabled" ,"true");
		else
			$this->callMethod("element.getButton('".$button."').removeAttribute", "disabled");
	}
}


/**
 * PrefWindow element
 * 
 * @link https://developer.mozilla.org/en/XUL/prefwindow
*/

class XULPrefWindow extends XULDialog
{
	public $remoteConstructor = 'PrefWindow';
	
	
	/**
	 * Makes the given pane active in the prefwindow
	 *
	 * @param XULPrefPane	$pane	Pane to activate
	 */
	
	public function showPane( $pane ) {
		if( ! $this->isRegistered )
			throw new SFException( "Can't call showPane() on unregistered PrefWindow" );
		
		$this->callMethod( 'showPane', array($pane) );
	}
}


/**
 * PrefPane element
 * 
 * @link https://developer.mozilla.org/en/XUL/prefpane
*/

class XULPrefPane extends BranchNode
{
	public $remoteConstructor = 'PrefPane';
	public $initAttributes = array( 'helpURI', 'label', 'image' );

	
	/**
	 * Dynamic Constructor
	 *
	 * @param string	$label
	 * @param string	$image
	 * @param mixed		$childNodes
	*/
	
	public function __construct() {
		if( func_num_args() ) {
			$args = func_get_args();
			
			if( count($args) && is_string($args[0]) )
				$this->label( array_shift($args) );
			if( count($args) && is_string($args[0]) )
				$this->image( array_shift($args) );
			
			parent::__construct( $args );
		}
		else parent::__construct();
	}
	
	
	public function helpURI( $href = NULL ) {
		return $this->attributeMethod( 'helpURI', (string) $href );
	}
	

	public function image( $src = NULL ) {
		if( $src === NULL )
			return (isset($this->image) ? $this->image : NULL);
		
		$this->image = $src;
		
		if( $this->isRegistered )
			$this->callMethod( 'image', $this->image );
		
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
	const BUTTON_POS_0 = 1;
    const BUTTON_POS_1 = 256;
    const BUTTON_POS_2 = 65536;


	const BUTTON_TITLE_OK = 1;
    const BUTTON_TITLE_CANCEL = 2;
    const BUTTON_TITLE_YES = 3;
    const BUTTON_TITLE_NO = 4;
    const BUTTON_TITLE_SAVE = 5;
    const BUTTON_TITLE_DONT_SAVE = 6;
    const BUTTON_TITLE_REVERT = 7;
    const BUTTON_TITLE_IS_STRING = 127;

    const BUTTON_POS_0_DEFAULT = 0;
    const BUTTON_POS_1_DEFAULT = 16777216;
    const BUTTON_POS_2_DEFAULT = 33554432;
	
	const BUTTON_DELAY_ENABLE = 67108864;
	const STD_OK_CANCEL_BUTTONS = 513;
	const STD_YES_NO_BUTTONS = 1027;
	
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
	 * Shows a alert box with an OK button
	 *
	 * @param string	$title	Title of the dialog box
	 * @param string	$text	Body text
	 */
	
	public function alert( $title, $text ) {
		$this->callMethod( 'alert', array($title,$text) );
	}
	
	
	/**
	 * Shows an alert box with an OK button and a checkbox
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function alertCheck( $title, $text, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'alertCheck', array($title,$text,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a confirm box with OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function confirm( $title, $text, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'confirm', array($title,$text) );
	}
	
	/**
	 * Shows a confirm box with OK and Cancel buttons and a checkbox
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function confirmCheck( $title, $text, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'confirmCheck', array($title,$text,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a custom confirm box with up to three buttons and a checkbox
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param int		$buttonFlags	Button flags
	 * @param string	$button0Title	Optional custom title for button 0
	 * @param string	$button1Title	Optional custom title for button 1
	 * @param string	$button2Title	Optional custom title for button 2
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 * 
	 * @link https://developer.mozilla.org/en/nsIPromptService#confirmEx example
	 */
	
	public function confirmEx( $title, $text, $buttonFlags, $button0Title, $button1Title, $button2Title, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'confirmEx', array($title,$text,$buttonFlags,$button0Title,$button1Title,$button2Title,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a prompt box with a textinput and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$textValue		Initial value of the textinput
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function prompt( $title, $text, $textValue, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'prompt', array($title,$text,$textValue,NULL,NULL) );
	}
	
	
	/**
	 * Shows a prompt box with a textinput, a checkbox and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$textValue		Initial value of the textinput
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptCheck( $title, $text, $textValue, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'prompt', array($title,$text,$textValue,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a prompt box with username and password textinputs and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$username		Initial value of the username textinput
	 * @param string	$password		Initial value of the password textinput
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptUsernameAndPassword( $title, $text, $username, $password, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'promptUsernameAndPassword', array($title,$text,$username,$password,NULL,NULL) );
	}
	
	
	/**
	 * Shows a prompt box with username and password textinputs, a checkbox and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$username		Initial value of the username textinput
	 * @param string	$password		Initial value of the password textinput
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptUsernameAndPasswordCheck( $title, $text, $username, $password, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'promptUsernameAndPassword', array($title,$text,$username,$password,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a prompt box with a password textinput and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$password		Initial value of the password textinput
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptPassword( $title, $text, $password, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'promptPassword', array($title,$text,$password,NULL,NULL) );
	}
	
	
	/**
	 * Shows a prompt box with a password textinput, a checkbox and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param string	$password		Initial value of the password textinput
	 * @param string	$checkMsg		Label text of the checkbox
	 * @param bool		$checkState		Initial state of the checkbox
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function promptPasswordCheck( $title, $text, $password, $checkMsg, $checkState, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
		$this->promptInProgress = TRUE;
		$this->handlerObj = $handlerObj;
		$this->handlerMethod = $handlerMethod;
		$this->carryValue = $carryValue;
		$this->callMethod( 'promptPassword', array($title,$text,$password,$checkMsg,$checkState) );
	}
	
	
	/**
	 * Shows a prompt box with a dropdown box and OK and Cancel buttons
	 *
	 * @param string	$title			Title of the dialog box
	 * @param string	$text			Body text
	 * @param array		$list			Array of options (strings)
	 * @param int		$selectedIndex	Initial index to select (broken)
	 * @param object	$handlerObj		Object to call the handler method on
	 * @param string	$handlerMethod	Method to call on the handler object
	 * @param mixed		$carryValue		Arbitrary value to pass on to the handler function
	 */
	
	public function select( $title, $text, $list, $selectedIndex, $handlerObj, $handlerMethod, $carryValue = NULL ) {
		if( $this->promptInProgress ) throw new SFException( 'PromptService: a prompt is already in progress', ERR_REPORT_APP );
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


/**
 * OS-native alert notification node
 * 
 * This node displays taskbar style notification boxes in Windows and Growl notifications in MacOS X (when installed).
 * Alert notifications can be made clickable, making them fire 'command' events when clicked. They also fire a 'finished'
 * event when disappearing. The notification appears as soon as it's registered (added to a registered parent node).
 */

class AlertNotification extends Node
{
	public $remoteConstructor = 'AlertNotification';
	
	
	/**
	 * Constructor
	 *
	 * @param string	$image			Path to the image to display (empty string to display no image)
	 * @param string	$title			Title of the alert
	 * @param string	$text			Text of the alert
	 * @param string	$name			Name of the notification type (if supplied, this can be used by f.e. Growl to disable certain messages)
	 * @param bool		$textClickable	Boolean indicating whether the notification should be clickable. If TRUE, the notification fires a 'command' event when clicked.
	 */
	
	public function __construct( $image, $title, $text, $name = NULL, $textClickable = FALSE ) {
		$this->image = $image;
		$this->title = $title;
		$this->text = $text;
		$this->name = $name;
		$this->textClickable = $textClickable;
	}
	
	public function attach() {
		parent::attach();
		$this->setEvent( 'finished', MSG_SEND, $this, 'onFinished' );
		
		$this->callMethod( 'showAlertNotification', array( $this->image, $this->title, $this->text, $this->name, $this->textClickable ) );
	}
	
	
	/**
	 * Internal handler for the 'finished' event, removes the node
	 */
	
	public function onFinished( $e ) {
		$this->removeNode();
	}
}
