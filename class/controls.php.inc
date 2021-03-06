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
 * @subpackage Controls
 */
/**
 * Button element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/button
 */
class XULButton extends BranchNode
{

    public $label;
    public $disabled;
    public $remoteConstructor = 'Button';
    public $initAttributes = array('image');

    /**
     * Dynamic Constructor
     *
     * @param string	$label
     * @param int		$width
     * @param int		$height
     * @param string	$image
     * @param mixed		$childNodes
     */
    public function __construct() {
        if (func_num_args()) {
            $args = func_get_args();

            if (count($args) && is_string($args[0]))
                $this->label(array_shift($args));
            if (count($args) && is_int($args[0]))
                $this->width(array_shift($args));
            if (count($args) && is_int($args[0]))
                $this->height(array_shift($args));
            if (count($args) && is_string($args[0]))
                $this->image(array_shift($args));

            parent::__construct($args);
        }
        else
            parent::__construct();
    }

    public function attach() {
        parent::attach();
        $this->setEventType('command', MSG_SEND);
    }

    /**
     * Sets an image path
     *
     * The path can either be a URL or a local path starting with /
     * In the case of a local path, the root is considered to be the sitefusion directory, so
     * the path "/app/testapp/myimage.png" points to this path within your sitefusion directory
     *
     * @param string	$src	The URL or path to the image
     * @return object|string	Returns a reference to the self for chaining method calls, or the current image value if called without arguments
     */
    public function image($src = NULL) {
        if ($src === NULL)
            return (isset($this->image) ? $this->image : NULL);

        $this->image = $src;

        if ($this->isRegistered)
            $this->callMethod('image', $this->image);

        return $this;
    }
}

/**
 * Progressmeter element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/progressmeter
 */
class XULProgressMeter extends Node
{

    public $remoteConstructor = 'ProgressMeter';

    public function __construct($mode = 'determined', $value = 0) {
        $this->mode($mode);
        if ($mode == 'determined')
            $this->value($value);

        parent::__construct();
    }

    public function attach() {
        $this->createRemoteObject(array($this->hostWindow, $this->mode));
        $this->insertElement();
    }

    /**
     * Sets the progress mode
     *
     * The supplied mode can be:
     * - 'determined': the progress meter is determined and shows the value set through the value() method
     * - 'undetermined': the progress meter shows undetermined activity
     *
     * @param string	$src	The URL or path to the image
     * @return object|string	Returns a reference to the self for chaining method calls, or the current mode if called without arguments
     */
    public function mode($mode = NULL) {
        return $this->attributeMethod('mode', ($mode == 'determined' ? 'determined' : 'undetermined'));
    }
}

/**
 * Splitter element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/splitter
 */
class XULSplitter extends BranchNode
{

    public $remoteConstructor = 'Splitter';
    protected $className;

    /*
     * Dynamic Constructor
     *
     * @param string	$className	Class name for styling ('tree-splitter' for splitters between tree columns)
     * @param mixed		$childNodes
     */

    public function __construct() {
        if (func_num_args()) {
            $args = func_get_args();

            if (count($args) && is_string($args[0]))
                $this->className = array_shift($args);

            parent::__construct($args);
        }
        else
            parent::__construct();
    }

    public function attach() {
        parent::attach();
        $this->setAttribute('class', $this->className);
    }
}

/**
 * Colorpicker element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/colorpicker
 */
class XULColorPicker extends Node
{

    public $remoteConstructor = 'ColorPicker';
    public $initAttributes = array('color', 'type');

    public function __construct($color = NULL, $type = NULL) {
        if ($color !== NULL)
            $this->color($color);
        if ($type !== NULL)
            $this->type($type);

        parent::__construct();
    }

    public function attach() {
        $this->createRemoteObject(array($this->hostWindow));
        $this->insertElement();
        $this->setEvent('yield', MSG_QUEUE, $this, 'yieldCollect');
    }

    /**
     * Set and get the selected color
     *
     * @param string	$color		Color in hexadecimal HTML format (#xxxxxx)
     * @return string	Returns the current value when called without arguments, otherwise returns a reference to self for method chaining
     */
    public function color($color = NULL) {
        return $this->attributeMethod('color', $color);
    }

    /**
     * Set and get the type attribute of the color picker
     *
     * @link https://developer.mozilla.org/en/XUL/colorpicker#a-colorpicker.type
     * @param string	$type		Use 'button' to display a button with a popup colorpicker
     * @return string	Returns the current value when called without arguments, otherwise returns a reference to self for method chaining
     */
    public function type($type = NULL) {
        return $this->attributeMethod('type', $type);
    }

    public function yieldCollect($e, $color) {
        $this->color = $color;
    }
}

/**
 * Scale element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/scale
 */
class XULScale extends Node
{

    public $remoteConstructor = 'Scale';
    public $initAttributes = array('min', 'max', 'value', 'increment');

    public function __construct($min = NULL, $max = NULL, $value = NULL) {
        if ($min !== NULL)
            $this->min($min);
        if ($max !== NULL)
            $this->max($max);
        if ($value !== NULL)
            $this->value($value);

        parent::__construct();
    }

    public function attach() {
        parent::attach();
        $this->setEvent('yield', MSG_QUEUE, $this, 'yieldCollect');
    }

    public function min($min = NULL) {
        return $this->attributeMethod('min', $min);
    }

    public function max($max = NULL) {
        return $this->attributeMethod('max', $max);
    }

    public function value($value = NULL) {
        return $this->attributeMethod('value', $value);
    }

    public function increment($increment = NULL) {
        return $this->attributeMethod('increment', $increment);
    }

    public function yieldCollect($e, $value = NULL) {
        $this->value = $value;
    }
}

/**
 * Textbox element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/textbox
 */
class XULTextBox extends BranchNode
{

    private $range = -2147483647;
    public $validity;
    public $remoteConstructor = 'TextBox';
    public $initAttributes = array('multiline', 'readonly', 'type', 'cols', 'rows',
        'timeout', 'wrap', 'maxlength', 'min', 'max', 'searchbutton', 'increment',
        'emptytext', 'hidespinbuttons', 'decimalplaces', 'newlines', 'wraparound');

    public function __construct($value = '', $multiline = FALSE, $width = NULL, $height = NULL) {
        $this->multiline($multiline);
        $this->value($value);
        $this->width($width);
        $this->height($height);

        parent::__construct();
    }

    public function attach() {
        parent::attach();
        $this->setEventHandler('yield', $this, 'yieldCollect');
    }

    public function yieldCollect($e, $val, $validity) {
        $this->validity = json_decode($validity);
        $this->value = $val;
    }

    public function multiline($state = NULL) {
        return $this->attributeMethod('multiline', ($state === FALSE || $state === 'false' ? 'false' : 'true'));
    }

    public function readonly($state = NULL) {
        return $this->attributeMethod('readonly', ($state === FALSE || $state === 'false' ? 'false' : 'true'));
    }

    public function type($val = NULL) {
        return $this->attributeMethod('type', $val);
    }

    public function cols($val = NULL) {
        return $this->attributeMethod('cols', $val);
    }

    public function rows($val = NULL) {
        return $this->attributeMethod('rows', $val);
    }

    public function timeout($val = NULL) {
        return $this->attributeMethod('timeout', $val);
    }

    public function wrap($val = NULL) {
        return $this->attributeMethod('wrap', $val);
    }

    public function maxlength($val = NULL) {
        return $this->attributeMethod('maxlength', $val);
    }

    public function min($val = NULL) {
        return $this->attributeMethod('min', $val);
    }

    public function max($val = NULL) {
        return $this->attributeMethod('max', $val);
    }

    public function searchbutton($val = NULL) {
        return $this->attributeMethod('searchbutton', $val);
    }

    public function increment($val = NULL) {
        return $this->attributeMethod('increment', $val);
    }

    public function emptytext($val = NULL) {
        return $this->attributeMethod('emptytext', $val);
    }

    public function hidespinbuttons($val = NULL) {
        return $this->attributeMethod('hidespinbuttons', $val);
    }

    public function decimalplaces($val = NULL) {
        return $this->attributeMethod('decimalplaces', $val);
    }

    public function newlines($val = NULL) {
        return $this->attributeMethod('newlines', $val);
    }

    public function wraparound($val = NULL) {
        return $this->attributeMethod('wraparound', $val);
    }

    public function unsigned($signed = false) {
        return $this->min(( $signed ? 0 : $this->range));
    }

    //visual feedback validation functions

    public function setInputFieldType($type) {
        $this->callMethodDelayed("element.inputField.setAttribute", array('type', $type), 100);
    }

    public function setInputFieldRequired($state) {
        if ($state)
            $this->callMethodDelayed("element.inputField.setAttribute", array('required', TRUE), 100);
        else
            $this->callMethodDelayed("element.inputField.removeAttribute", array('required'), 100);
    }

    public function setInputFieldPlaceholder($placeholder) {
        $this->callMethodDelayed("element.inputField.setAttribute", array('placeholder', $placeholder), 100);
    }

    public function setInputFieldPattern($pattern) {
        $this->callMethodDelayed("element.inputField.setAttribute", array('pattern', $pattern), 100);
    }

    //retrieves validity object after yield
    public function getValidity() {
        return $this->validity;
    }

    //autocompletion attribute setters

    public function enablehistory($val = NULL) {
        return $this->attributeMethod('enablehistory', ($val ? "true" : ("false")));
    }

    public function highlightnonmatches($val = NULL) {
        return $this->attributeMethod('highlightnonmatches', ($val ? "true" : ("false")));
    }

    public function completedefaultindex($val = NULL) {
        return $this->attributeMethod('completedefaultindex', ($val ? "true" : ("false")));
    }

    public function showcommentcolumn($val = NULL) {
        return $this->attributeMethod('showcommentcolumn', ($val ? "true" : ("false")));
    }

    public function autocompletesearch($val = NULL) {
        return $this->attributeMethod('autocompletesearch', $val);
    }

    public function autocompletepopup($node = NULL) {
        if (is_object($node) && $node instanceof XULPanel) {
            return $this->attributeMethod('autocompletepopup', "cid" . $node->id);
        } else if (is_object($node) && $node instanceof XULMenuPopup) {
            return $this->attributeMethod('autocompletepopup', "cid" . $node->id);
        }
        else
            return false;
    }

    public function autocompletesearchparam($val = NULL) {

        if (is_string($val))
            return $this->attributeMethod('autocompletesearchparam', $val);
        else if (is_array($val)) {
            $retArray = array();
            foreach ($val as $key => $value) {
                $retArray[] = array("value" => $key, "comment" => $value);
            }
            return $this->attributeMethod('autocompletesearchparam', json_encode($retArray));
        }
        else
            return FALSE;
    }

}

/**
 * Timepicker element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/timepicker
 */
class XULTimePicker extends Node
{
    public $remoteConstructor = 'TimePicker';

    /**
     * Dynamic constructor
     * @param mixed $value  The value to set, this is either the unix timestamp or a hh:mm:ss time
     * @param int   $width  The initial height
     * @param int   $height The initial width
     */
    public function __construct($value = '', $width = NULL, $height = NULL) {
        $this->value($value);
        $this->width($width);
        $this->height($height);

        parent::__construct();
    }

    public function attach() {
        parent::attach();
        $this->setEventHandler('yield', $this, 'yieldCollect');
    }

    public function yieldCollect($e, $val, $hour, $minute, $second, $timestamp) {
        $this->value = $val;
        $this->hour = $hour;
        $this->minute = $minute;
        $this->second = $second;
        $this->timestamp = $timestamp;
    }

    /**
     * Resets the date value to its default
     */
    public function reset() {
        $this->callMethod('reset');
    }

    public function readonly($val = NULL) {
        return $this->attributeMethod('readonly', $val);
    }

    public function increment($val = NULL) {
        return $this->attributeMethod('increment', $val);
    }
}

/**
 * Datepicker element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/datepicker
 */
class XULDatePicker extends Node
{
    public $remoteConstructor = 'DatePicker';
    public $initAttributes = array('type');

    /**
     * Dynamic constructor
     * @param string   $type  Defines the display type, this should be either 'normal', 'grid' or 'popup'
     * @param mixed    $value The value to set, this is either the unix timestamp or a yyyy-mm-dd date
     */
    public function __construct($type = NULL, $value = NULL) {
        if ($type !== NULL) {
            $this->type($type);
        }

        if (is_int($value)) {
            $this->value(date('Y-m-d', $value));
        } elseif ($value === NULL) {
            $this->value(date('Y-m-d'));
        } else {
            $this->value($value);
        }

        $this->setEventHandler('yield', $this, 'yieldCollect');

        parent::__construct();
    }

    public function yieldCollect($e, $value = NULL) {
        $this->value = $value;
    }

    /**
     * Resets the date value to its default
     */
    public function reset() {
        $this->callMethod('reset');
    }

    /**
     * Defines the display type, this should be either 'normal', 'grid' or 'popup'
     * @param  string $type The display type
     * @return mixed        Returns the current value or reference to self ($this) for easy chaining of method calls
     */
    public function type($type = NULL) {
        return $this->attributeMethod('type', $type);
    }

    /**
     * Sets the value
     * @param  mixed $val The value to set, this is either the unix timestamp or a yyyy-mm-dd date
     * @return mixed      Returns the current value or reference to self ($this) for easy chaining of method calls
     */
    public function value($val = NULL) {
        if ($val === NULL) {
            return (isset($this->value) ? $this->value : NULL);
        }

        $this->value = $val;

        if ($this->isRegistered) {
            $this->callMethod( 'value', array($this->value) );
        }

        return $this;
    }
}

/**
 * Checkbox element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/checkbox
 */
class XULCheckBox extends BranchNode
{

    public $remoteConstructor = 'CheckBox';
    public $checked = FALSE;
    public $initAttributes = array('checked', 'src');

    /**
     * Dynamic Constructor
     *
     * @param string	$label
     * @param string	$image
     * @param bool		$checked
     * @param mixed		$childNodes
     */
    public function __construct() {
        if (func_num_args()) {
            $args = func_get_args();

            if (count($args) && is_string($args[0]))
                $this->label(array_shift($args));
            if (count($args) && is_string($args[0]))
                $this->src(array_shift($args));
            if (count($args) && is_bool($args[0]))
                $this->checked(array_shift($args));

            parent::__construct($args);
        }
        else
            parent::__construct();

        $this->setEventHandler('yield', $this, 'yieldCollect');
    }

    public function yieldCollect($e, $state) {
        $this->checked = $state;
    }

    public function checked($state = NULL) {
        return $this->attributeMethod('checked', $state);
    }

    public function src($src = NULL) {
        return $this->attributeMethod('src', $src);
    }
}

/**
 * Radiogroup element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/radiogroup
 */
class XULRadioGroup extends BranchNode
{
    public $remoteConstructor = 'RadioGroup';
    public $orient = 'vertical';
    public $value = NULL;
    public $selectedItem = NULL;
    public $selectAfterAttach = NULL;

    /**
     * Dynamic Constructor
     *
     * @param mixed		$childNodes
     */
    public function __construct() {
        $args = func_get_args();
        parent::__construct($args);
    }

    public function attach() {
        parent::attach();
        if ($this->selectAfterAttach != NULL) {
            $this->selectItem($this->selectAfterAttach);
            $this->selectAfterAttach = NULL;
        }

        $this->setEventHandler('yield', $this, 'yieldCollect');
    }

    public function yieldCollect($e, $item) {
        $this->selectedItem = $item;
        $this->value = ($item != NULL ? $item->value : NULL);
    }

    /* Deselects all child radio's  */

    public function clearSelection() {
        $this->setProperty("element.selectedIndex", -1);
    }

    public function selectItem($val) {
        if (!$this->isRegistered) {
            $this->selectAfterAttach = $val;
            return;
        }

        if ($val instanceof XULRadio) {
            foreach ($this->children as $item) {
                if ($item instanceof XULRadio && $item === $val) {
                    $item->selected(TRUE);

                    break;
                }
            }
        } else if ($val === NULL) {
            $this->selectedItem = $val;
            $this->value = $val;

            foreach ($this->children as $item) {
                if ($item instanceof XULRadio && $item->selected()) {
                    $item->selected(FALSE);
                }
            }
        } else {
            foreach ($this->children as $item) {
                if ($item instanceof XULRadio && $item->value() == $val) {
                    $item->selected(TRUE);
                }
            }
        }

        return $this;
    }

}

/**
 * Radio element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/radio
 */
class XULRadio extends BranchNode
{

    public $remoteConstructor = 'Radio';
    public $value = NULL;
    public $selected = FALSE;

    /**
     * Dynamic Constructor
     *
     * @param string	$label
     * @param bool		$selected
     * @param mixed		$value
     * @param mixed		$childNodes
     */
    public function __construct() {
        if (func_num_args()) {
            $args = func_get_args();

            if (count($args) && is_string($args[0]))
                $this->label(array_shift($args));
            if (count($args) && is_bool($args[0]))
                $this->selected(array_shift($args));
            if (count($args) && !(is_object($args[0]) && $args[0] instanceof Node))
                $this->value(array_shift($args));

            parent::__construct($args);
        }
        else
            parent::__construct();

        $this->setEventHandler('onAfterAttach', $this, 'setGroupValues');
    }

    public function setGroupValues() {
        if ($this->isChild && ($parent = $this->findAncestor('XULRadioGroup')) && $this->selected) {
            $parent->selectedItem = $this;
            $parent->value = $this->value;

            foreach ($parent->children as $radio) {
                if ($radio instanceof XULRadio && $radio !== $this)
                    $radio->selected = FALSE;
            }

            if ($this->isRegistered)
                $parent->callMethod('selectItem', array($this));
        }
    }

    public function value($val = NULL) {
        if ($val === NULL)
            return $this->value;

        $this->value = $val;

        return $this;
    }

    public function selected($state = NULL) {
        if ($state === NULL)
            return $this->selected;

        $this->selected = (bool) $state;

        $this->setGroupValues();

        return $this;
    }
}

/**
 * Menulist element
 *
 * @package API
 * @subpackage Controls
 * @link https://developer.mozilla.org/en/XUL/menulist
 */
class XULMenuList extends BranchNode
{

    /**
     * Added to facilitate the selectItem before the XULMenuList was registered to the DOM Tree
     * This will call selectItem in the attach function to select on attach and the reset the var
     * so it does not keep selecting the item on reattaching
     */
    private $selectAfterAttach = NULL;
    public $remoteConstructor = 'MenuList';
    public $initAttributes = array('editable');
    public $selectedItem = NULL;
    public $selectedValue = NULL;
    public $inputField = NULL;

    /**
     * Dynamic Constructor
     *
     * @param mixed		$childNodes
     */
    public function __construct() {
        $args = func_get_args();
        parent::__construct($args);
    }

    public function attach() {
        parent::attach();
        if ($this->selectAfterAttach != NULL) {
            $this->selectItem($this->selectAfterAttach);
            $this->selectAfterAttach = NULL;
        }

        $this->setEventHandler('yield', $this, 'yieldCollect');
    }

    public function yieldCollect($e, $item, $inputField = NULL) {
        $this->selectedItem = $item;
        $this->selectedValue = ($item ? $item->value() : NULL);
        $this->inputField = $inputField;
    }

    public function editable($state) {
        return $this->attributeMethod('editable', (bool) $state);
    }

    public function selectItem($val) {
        if (!(isset($this->children[0]) && $this->children[0] instanceof XULMenuPopup)) {
            throw new SFException('XULMenuList needs a XULMenuPopup childnode to contain menu items', ERR_REPORT_APP);
        }

        if (!$this->isRegistered) {
            $this->selectAfterAttach = $val;
            return;
        }

        if (is_integer($val)) {
            $this->selectedItem = $this->children[0]->children[$val];
            $this->selectedValue = $this->selectedItem->value();

            if ($this->isRegistered) {
                $this->callMethod('selectedItem', $val);
            }
        } elseif ($val instanceof XULMenuItem) {
            $n = -1;
            foreach ($this->children[0]->children as $item) {
                $n++;

                if ($item === $val) {
                    $this->selectedItem = $item;
                    $this->selectedValue = $item->value();

                    if ($this->isRegistered) {
                        $this->callMethod('selectedItem', $n);
                    }

                    break;
                }
            }
        } elseif ($val === NULL) {
            $this->selectedItem = $val;
            $this->selectedValue = $val;

            if ($this->isRegistered) {
                $this->callMethod('selectedItem', $val);
            }
        } else {
            foreach ($this->children[0]->children as $item) {
                if ($item->value() == $val) {
                    return $this->selectItem($item);
                }
            }
        }

        return $this;
    }

    public function selectedItem($val) {
        return $this->selectItem($val);
    }
}

