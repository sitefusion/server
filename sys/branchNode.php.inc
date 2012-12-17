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
 * @subpackage Node
 */

/**
 * This class represents all nodes that are capable of having childnodes
 * 
 * It adds methods for adding, inserting, removing and extracting nodes.
 */
class BranchNode extends Node {

    public $children = array();

    /**
     * Add one or more nodes to this node's child collection
     * 
     * @param mixed	$nodes		One or more nodes or array(s) of nodes to add
     * @return mixed	Returns a reference to self ($this) for easy chaining of method calls
     */
    public function addChild() {
        $args = func_get_args();

        for ($n = 0; $n < sizeof($args); $n++) {
            if (is_array($args[$n])) {
                if (count($args[$n]))
                    array_splice($args, $n, 1, $args[$n]);
                else
                    continue;
            }

            $node = $args[$n];

            if (!$node instanceof Node)
                throw new SFException('Object is not a Node', ERR_REPORT_APP);

            if ($node->isChild)
                throw new SFException('Node is already a child', ERR_REPORT_APP);

            $this->fireLocalEvent('onBeforeAddChild', array($node));

            array_push($this->children, $node);
            $node->isChild = TRUE;
            $node->parent = $this;

            if ($this->isRegistered) {
                $this->rootApplication->registry->add($this, $node);

                if ($node instanceof BranchNode)
                    $node->registerChildren();

                $node->fireLocalEvent('onAfterAttach');
            }

            $this->fireLocalEvent('onAfterAddChild', array($node));
        }

        return $this;
    }

    /**
     * Add a new childnode before an existing one
     * 
     * @param Node	$node		New node
     * @param Node	$childnode	Existing node before which to insert the new node
     * @return mixed	Returns a reference to self ($this) for easy chaining of method calls
     */
    public function addChildBefore($node, $childnode) {
        if ($node->isChild) {
            throw new SFException('Node is already a child', ERR_REPORT_APP);
            return FALSE;
        }
        if (!in_array($childnode, $this->children, TRUE)) {
            throw new SFException('Given childnode is not a child of this node', ERR_REPORT_APP);
            return FALSE;
        }

        $this->fireLocalEvent('onBeforeAddChild', array($node));

        $index = array_search($childnode, $this->children, TRUE);
        array_splice($this->children, $index, 0, array($node));
        $node->isChild = TRUE;
        $node->parent = $this;
        if ($this->isRegistered) {
            $node->insertBefore = $childnode;

            $this->rootApplication->registry->add($this, $node);

            if ($node instanceof BranchNode)
                $node->registerChildren();

            $node->fireLocalEvent('onAfterAttach');
        }

        $this->fireLocalEvent('onAfterAddChild', array($node));

        return $this;
    }

    /**
     * Add a new childnode after an existing one
     * 
     * @param Node	$node		New node
     * @param Node	$childnode	Existing node after which to insert the new node
     * @return mixed	Returns a reference to self ($this) for easy chaining of method calls
     */
    public function addChildAfter($node, $childnode) {
        if ($node->isChild) {
            throw new SFException('Node is already a child', ERR_REPORT_APP);
            return FALSE;
        }
        if (!in_array($childnode, $this->children, TRUE)) {
            throw new SFException('Given childnode is not a child of this node', ERR_REPORT_APP);
            return FALSE;
        }

        $index = array_search($childnode, $this->children, TRUE);
        if ($index == (count($this->children) - 1))
            $this->addChild($node);
        else
            $this->addChildBefore($node, $this->children[$index + 1]);

        return $this;
    }

    /**
     * Adds one or more new childnodes in front of all existing childnodes
     * 
     * @param mixed	$nodes		One or more nodes or array(s) of nodes to add
     * @return mixed	Returns a reference to self ($this) for easy chaining of method calls
     */
    public function addChildFirst() {
        $args = func_get_args();

        if (count($this->children)) {
            for ($n = count($args) - 1; $n >= 0; $n--) {
                if (is_array($args[$n])) {
                    $num = count($args[$n]);
                    array_splice($args, $n, 1, $args[$n]);
                    $n += $num - 1;
                }

                $obj = $args[$n];
                $this->addChildBefore($obj, $this->children[0]);
            }
        } else {
            $this->addChild($args);
        }

        return $this;
    }

    /**
     * [INTERNAL FUNCTION] Registers childnodes, called when this node is registered
     */
    public function registerChildren() {

        foreach ($this->children as $child) {
            $event = FALSE;

            if (!$child->isRegistered) {
                $this->rootApplication->registry->add($this, $child);
                $event = TRUE;
            }

            if (method_exists($child, 'registerChildren'))
                $child->registerChildren();

            if ($event)
                $child->fireLocalEvent('onAfterAttach');
        }
    }

    /**
     * Removes one or more childnodes from this node's child collection
     * 
     * Use this method if you don't intend to reuse the node(s)
     * 
     * @see Node::removeNode()
     * @param mixed	$nodes		One or more nodes or array(s) of nodes to remove
     * @return bool	Whether the operation was successful
     */
    public function removeChild() {

        $args = func_get_args();
        $return = array();

        for ($n = 0; $n < sizeof($args); $n++) {
            if (is_array($args[$n]))
                array_splice($args, $n, 1, $args[$n]);

            $obj = $args[$n];

            if (( $index = array_search($obj, $this->children, TRUE) ) === FALSE)
                ( 'Node is not a child of this BranchNode' );

            $this->fireLocalEvent('onBeforeRemoveChild', array($obj));

            if ($obj instanceof BranchNode && $this->isRegistered)
                $obj->unregisterChildren();

            $obj->unlink();

            array_splice($this->children, $index, 1);

            if ($this->isRegistered)
                $this->rootApplication->registry->remove($obj);

            $obj->unbind();

            $this->fireLocalEvent('onAfterRemoveChild', array($obj));

            array_push($return, $obj);
        }

        return TRUE;
    }

    /**
     * Extracts one or more childnodes from this node's child collection
     * 
     * Use this method if you intend to reuse the node(s), this will unset every event handler bound to the node(s)
     * 
     * @see Node::extractNode()
     * @param mixed	$nodes		One or more nodes or array(s) of nodes to remove
     * @return Node|array	The removed node or array of nodes
     */
    public function extractChild() {

        $args = func_get_args();
        $return = array();

        for ($n = 0; $n < sizeof($args); $n++) {
            if (is_array($args[$n]))
                array_splice($args, $n, 1, $args[$n]);

            $obj = $args[$n];

            if (( $index = array_search($obj, $this->children, TRUE) ) === FALSE)
                throw new SFException('Node is not a child of this BranchNode', ERR_REPORT_APP);

            $this->fireLocalEvent('onBeforeRemoveChild', array($obj));

            if ($obj instanceof BranchNode && $this->isRegistered)
                $obj->unregisterChildren();

            $obj->unlink();

            array_splice($this->children, $index, 1);

            if ($this->isRegistered)
                $this->rootApplication->registry->remove($obj);

            $obj->isChild = FALSE;
            unset($obj->parent);

            $this->fireLocalEvent('onAfterRemoveChild', array($obj));

            array_push($return, $obj);
        }

        return ( sizeof($return) == 1 ? $return[0] : $return );
    }

    /**
     * [INTERNAL FUNCTION] Registers childnodes, called when this node is registered
     */
    public function unregisterChildren() {

        foreach ($this->children as $child) {
            if ($child instanceof BranchNode)
                $child->unregisterChildren();

            $child->unlink();

            $this->rootApplication->registry->removeSilent($child);
        }
    }

    /**
     * Removes all childnodes from this node's child collection
     * 
     * Use this method if you don't intend to reuse the removed nodes
     * 
     * @see Node::removeNode()
     * @return bool	Whether the operation was successful
     */
    public function removeChildren() {
        if (count($this->children))
            return $this->removeChild($this->children);
    }

    /**
     * Extracts all childnodes from this node's child collection
     * 
     * Use this method if you intend to reuse the nodes
     * 
     * @see Node::extractNode()
     * @return Node|array	The removed node or array of nodes
     */
    public function extractChildren() {
        if (count($this->children))
            return $this->extractChild($this->children);
    }

    /**
     * Get the index of a child node
     * 
     * Returns the index of the given childnode (as in the $children array) or NULL if not found
     * 
     * @return int|NULL	The index of the childnode or NULL if not found
     */
    public function findChild($child) {
        for ($n = 0; $n < count($this->children); $n++) {
            if ($child === $this->children[$n])
                return $n;
        }

        return NULL;
    }

    /**
     * Recursively collects all nodes under this node
     * 
     * Examples:
     * 
     * Return all nodes under $node:
     * $node->getDescendants();
     * 
     * Return all nodes of class XULTextBox:
     * $node->getDescendants( 'XULTextBox' );
     * 
     * Return all nodes of either the XULTextBox or XULCheckbox:
     * $node->getDescendants( array('XULTextBox','XULCheckbox') );
     * 
     * Return all checkboxes that are checked:
     * $node->getDescendants( 'XULCheckbox', array( 'checked' => TRUE ) );
     * 
     * @param string|array	$className		When supplied, only nodes of the given class(es) are returned
     * @param array			$properties		Optional associative array of property names and values that nodes are checked against
     * @param bool			$strict			Apply strict comparison (===) of the given properties
     */
    public function getDescendants($className = NULL, $properties = NULL, $strict = FALSE) {
        $collection = array();
        $childCollection = array();

        foreach ($this->children as $child) {
            if ($child instanceof BranchNode)
                $childCollection = array_merge($childCollection, $child->getDescendants($className, $properties, $strict));

            if ($className !== NULL) {
                if (is_string($className)) {
                    if (!$child instanceof $className)
                        continue;
                }
                elseif (is_array($className)) {
                    $match = FALSE;
                    foreach ($className as $cn) {
                        if ($child instanceof $cn)
                            $match = TRUE;
                    }
                    if (!$match)
                        continue;
                } else
                    continue;
            }

            if ($properties !== NULL) {
                $match = TRUE;
                foreach ($properties as $name => $value) {
                    if (!(isset($child->{$name}) && ($strict ? ($child->{$name} === $value) : ($child->{$name} == $value)) )) {
                        $match = FALSE;
                        break;
                    }
                }
                if (!$match)
                    continue;
            }

            $collection[] = $child;
            if (count($childCollection)) {
                $collection = array_merge($collection, $childCollection);
                $childCollection = array();
            }
        }

        return array_merge($collection, $childCollection);
    }

}
