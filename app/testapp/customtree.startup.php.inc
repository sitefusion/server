<?php

function authorizeLogin( $args, $user, $pass ) {
	// We'll just hardcode these credentials here, normally you'd want to do something
	// more sophisticated here...
	return ($user == 'user' && $pass == 'password');
}

function getApplication( $args ) {
	// Return the name of the application class that should be started.
	return 'SFCustomTreeDemo';
}


class SFCustomTreeDemo extends Application
{
	public function init( $event ) {
		
		// Set the window title and size
		$this->window->title( 'CustomTree Demo' );
		$this->window->size( 1024, 740 );
		
		// Basic user interface construction. $this->window is the root window, and the
		// only registered element available when this function is called. It is used to
		// control the window, and as a container for the rest of the interface elements 
		// using the addChild() method or its variants. See the documentation on the
		// BranchNode class for more information.
		//
		// Most elements (XULxxxx classes) have 'Dynamic Constructors', for lack of a 
		// better word. These constructors don't have fixed arguments, but allow you to
		// set the most used XUL attribute values for the node, and to add child elements
		// all in the same constructor. For a list of the attributes that can be set with 
		// the constructor of a certain element, look up the documentation on that class 
		// at http://www.sitefusion.org/documentation
		//
		// For more information on what XUL elements are and what they do, check out the 
		// Mozilla XUL documentation at https://developer.mozilla.org/en/XUL
		// 
		// In this case, we'll be constructing two separate trees, with a splitter in 
		// between. The left tree will contain a hierarchical data structure of foods, in
		// which foods can be dragged between categories. The right tree will be a flat
		// ordered view. It will be empty initially, it will add dragged rows from the 
		// left tree to its list and it will enable dragging of its own rows to allow a 
		// manual order of rows.
		
		$this->window->addChild(
			new XULHBox( 1,
				$this->leftTree = new XULCustomTree( 1,
					new XULTreeCols(
						// In these column constructors, the first string is the display
						// name, the integer is the flex value and the last string is the
						// data id (will be used by TreeDataSet to determine which data
						// variable to place in the column). The boolean TRUE in the name
						// column indicates that it is the primary column, allowing it to
						// display the dropmarkers.
						
						$lCheck = new XULTreeCol( 'Check', 1, 'check' ),
						$lName = new XULTreeCol( 'Name', 2, TRUE, 'name' ),
						$lDescr = new XULTreeCol( 'Description', 3, 'description' )
					),
					new XULTreeChildren
				),
				
				new XULSplitter,
				
				$this->rightTree = new XULCustomTree( 1,
					new XULTreeCols(
						$rName = new XULTreeCol( 'Name', 2, 'name' ),
						$rDescr = new XULTreeCol( 'Description', 3, 'description' )
					),
					new XULTreeChildren
				)
			)
		);
		
		// In order to make anything editable in tree, we need to mark the tree as a whole
		// as editable. This does not make anything editable by itself, columns and rows 
		// have to be marked editable separately.
		
		$this->leftTree->editable( TRUE );
		
		// Column types can be one of 'text', 'progressmeter' or 'checkbox'. In this case
		// we'll set the type of the Check column to 'checkbox' and make it editable. Note
		// how method calls can be chained to save you some typing. This is true for most
		// element methods.
		
		$lCheck->type( 'checkbox' )->editable( TRUE );
		
		// The default column type is text so this doesn't have to be set separately. We'll
		// make the name and description columns editable as well.
		
		$lName->editable( TRUE );
		$lDescr->editable( TRUE );
		
		// Using setEventHandler on the left tree we set handlers for the events 'cellUpdated',
		// which is called when a cell has been edited, and for 'treeItemDropped', which is 
		// called when a tree item is dropped in the tree.
		
		$this->leftTree->setEventHandler( 'cellUpdated', $this, 'leftCellUpdated' );
		$this->leftTree->setEventHandler( 'treeItemDropped', $this, 'leftTreeItemDropped' );
		
		// Here we'll construct the TreeDataSet that will contain the data displayed in the
		// tree. It takes the tree as its first argument. The second argument is a boolean
		// indicating whether the tree should be sparse. If the tree is sparse, it only sends
		// visible elements to the client and adds new visible rows when a container is 
		// expanded. This is especially useful for large trees, because it takes a lot less
		// time and traffic to load the initial display of the tree. If left unspecified, trees
		// are not sparse by default.
		
		$this->leftData = new TreeDataSet( $this->leftTree, FALSE );
		
		// With the setDraggable() method we indicate that the contents of this dataset can be
		// dragged (TRUE) and that the feedback cursor should indicate a 'move' operation. The
		// exact display of this cursor feedback depends on the platform of the client.
		// Possible values are 'move', 'copy', 'link' or 'moveOrCopy', which alternates between
		// move and copy depending on whether the shift key is pressed.
		
		$this->leftData->setDraggable( TRUE, 'move' );
		
		// Here we indicate that the left dataset will also accept dropped items (TRUE), and that
		// items can be dropped 'on' existing rows. Other options are 'between', where items can
		// only be dropped between rows, and 'any', where items can be dropped on as well as 
		// between rows.
		
		$this->leftData->allowDrop( TRUE, 'on' );
		
		// Here we indicate that the left dataset may be sorted by the user by clicking the 
		// column headers. The default sort algorithm is 'natural', but with the 
		// setColumnSortType( $col, $type ) method other sort algorithms can be set as well.
		// Possible sorting types are 'natural', 'numeric', 'alphanumeric' and 'date'.
		
		$this->leftData->setSortable( TRUE );
		
		// Here we'll attach a handler for the treeItemDropped event on the right tree. This handler
		// will insert rows dragged from the left tree and reorder rows dragged from within itself.
		
		$this->rightTree->setEventHandler( 'treeItemDropped', $this, 'rightTreeItemDropped' );
		
		// Create a second dataset for the right tree, not sparse by default.
		
		$this->rightData = new TreeDataSet( $this->rightTree );
		
		// Make its contents draggable and indicate a move action
		
		$this->rightData->setDraggable( TRUE, 'move' );
		
		// Allow dropping of items between existing rows
		
		$this->rightData->allowDrop( TRUE, 'between' );
		
		// Here we indicate that this tree will also accept rows from other trees. This is FALSE by
		// default, meaning that the right tree will accept rows dragged from the left tree but the
		// left tree will not accept rows dragged from the right tree.
		
		$this->rightData->allowForeignDrop( TRUE );
		
		
		// Here we'll define some data to display in the left tree.
		// First the root items
		
		$rootData = array(
			'Vegetables' => array( 'name' => 'Vegetables', 'description' => 'Quite Healthy' ),
			'Fruits' => array( 'name' => 'Fruits', 'description' => 'Nice and sweet!' ),
			'Meat' => array( 'name' => 'Meat', 'description' => 'Dont eat to much of it...' )
		);
		
		// Then the subitems
		
		$subData = array(
			'Vegetables' => array(
				array( 'name' => 'Arugula', 'description' => 'Arugula is an aromatic salad green. It is also known as rocket, roquette, rugula and rucola, and is popular in Italian cuisine.It is high in vitamins A and C.' ),
				array( 'name' => 'Basil', 'description' => 'This aromatic herb is native to India, SE Asia, and NE Africa and is an important herb used in Italian and Thai cooking (and less significantly in other cuisines).' ),
				array( 'name' => 'Green Beans', 'description' => 'Green beans are an excellent source of vitamin C, vitamin K and manganese. Plus green beans are very good source of vitamin A (notably through their concentration of carotenoids including beta-carotene), dietary fiber, potassium, folate, and iron.' ),
				array( 'name' => 'Beets', 'description' => 'Are colorful root vegetables containing powerful nutrient compounds that help protect against heart disease, birth defects and certain cancers, especially colon cancer. The greens attached to the beet roots are delicious and can be prepared like spinach or Swiss chard. They are incredibly rich in nutrients, concentrated in vitamins and minerals as well as carotenoids such as beta-carotene and lutein/zeaxanthin. ' )
			),
			'Fruits' => array(
				array( 'name' => 'ABIU (Pouteria caimito)', 'description' => 'Yellow fruit about the size of a tennis ball. Usually has a nipple on one end of the fruit. It is juicy with transparent flesh and a caramel like flavor. The tree is small to medium sized, 12 to 15 feet tall. It bears very heavily, usually in the winter time. Needs full sun and good drainage. Trees are fast growing and bear fruit in 2 to 3 years.' ),
				array( 'name' => 'BILIMBI (Averrhoa Bilimbi) ', 'description' => 'Close relative of the starfruit (see below). This fruit is shaped like a miniature cucumber and 2 to 3 inches long. It is yellow when ripe. Fruit is sour and used as a relish or for pickling. Tree is upright and decorative. Will get 15 to 20 feet and and 10 feet wide.' ),
				array( 'name' => 'YELLOW JABOTICABA OR CABELLUDA', 'description' => 'Small bushy tree from Brazil. Fruits look like fuzzy orange marbles. They are juicy and tasty with an apricot-like flavor.' ),
				array( 'name' => 'BLACKBERRY JAM FRUIT (Randia formosa)', 'description' => 'A bushy tree with fragrant white flowers. Fruits look like small feijoas, and are 1 inch long and 1/2 inch wide. They really do taste like a blackberry jam.' ),
				array( 'name' => 'BRAZILIAN CHERRY (Eugenia dombeyi)', 'description' => 'Fruit looks and tastes similar to a Bing Cherry. Very delicious. Small to medium sized bushy tree. Medium gowth rate, Usually bears in about three years. Starts bearing when small and bears profusely. Needs full sun and good drainage.' )
			),
			'Meat' => array(
				array( 'name' => 'BBQ Chicken Quarters', 'description' => 'we lightly season leg or breast quarters with our signature rub, then smoke ‘em nice and slow to juicy perfection with just hint of cherry wood smoke. Glazed with our signature sauce.' ),
				array( 'name' => 'St. Louis Ribs', 'description' => 'Meaty, tender, lip smackin’ ribs-- glazed, not drowned in sauce.' ),
				array( 'name' => 'Pulled Pork', 'description' => 'Seasoned and slow smoked to bring out the REAL pork flavor. You add the sauce.' ),
				array( 'name' => 'Sliced Beef Brisket', 'description' => 'Thinly sliced and smoked to tender perfection. You can sauce if you want, but awesome on its own!' )
			)
		);
		
		// Now we'll add the arrays to the left dataset
		
		foreach ( $rootData as $name => $data ) {
			// The array inside $data has keys corresponding with the data ID's supplied to the 
			// constructors of the XULTreeCol elements, so all we have to do is pass the arrays 
			// as the first argument, and a boolean indicating whether this row is going to be 
			// a container (TRUE in this case) as a second argument.
			$row = $this->leftData->createRow( $data, TRUE );
			
			// Add the row to the dataset
			$this->leftData->addRow( $row );
			
			// Make the 'check', 'name' and 'description' cells editable
			$row->setEditable( 'check', TRUE )->setEditable( 'name', TRUE )->setEditable( 'description', TRUE );
			
			// Now add the sub-rows
			foreach ( $subData[$name] as $data ) {
				// Here the subrow is created, omitting the second argument which defaults
				// to FALSE (not a container)
				$subrow = $this->leftData->createRow( $data );
				
				// Add this sub-row to its parent row
				$row->addRow( $subrow );
				
				// Make the 'check', 'name' and 'description' cells editable
				$subrow->setEditable( 'check', TRUE )->setEditable( 'name', TRUE )->setEditable( 'description', TRUE );
			}
		}
		
		// This is a workaround. If we want the right tree to be initially empty, but still accept 
		// dropped rows, it needs to have a placeholder row. It can be removed as soon as real rows
		// are dropped into it.
		$this->rightData->addRow( $this->placeHolderRow = $this->rightData->createRow( array( 'name' => '', 'description' => '' ) ) );
	}
	
	
	// This is the event handler for edited cells in the left tree. As you can see,
	// nothing needs to be done to save the value, it's already available under
	// the property of the row with the same name as the column data ID. This property
	// can be read and set. For instance, setting:
	// $row->name = 'foo';
	// will result in the contents of the 'Name' cell of this row being changed to 'foo'.
	
	public function leftCellUpdated( $e, $row, $cell, $value ) {
		// Here we just show an alert box about the change
		$this->window->alert( "Cell '$cell' is now: $value" );
	}
	
	
	// This is the event handler for the 'treeItemDropped' event on the left tree, and is called
	// when a row is dropped on a container row in the left tree. Event handlers always receive
	// the Event object as the first parameter, and the event arguments as the rest of the parameters.
	// The second parameter is in this case the tree from which the row(s) were dragged. Multiple 
	// rows can be dropped at once, they are passed to the handler in the third parameter ($rows). 
	// The row that the item(s) were dropped on is the fourth parameter ($targetRow).
	
	public function leftTreeItemDropped( $e, $tree, $rows, $targetRow ) {
		foreach ( $rows as $row ) {
			// Remove the rows from their original position and re-insert them in their
			// new container
			$targetRow->addRow( $row->remove() );
		}
	}
	
	
	// This is the event handler for the 'treeItemDropped' event on the right tree, and is called
	// when a row is dropped between existing rows in the right tree. Same arguments apply, but
	// here the fifth parameter contains the orientation of the drop with respect to the target row.
	// This orientation value can be -1 (row was dropped above the target row), 1 (row was dropped
	// below the target row), or 0 (row was dropped on the target row, not possible for this tree 
	// since it only accepts drops 'between' rows).
	// This tree accepts dragged rows from itself as well as from the left tree, so it needs to make
	// a distinction here. This can be done with the $tree parameter, a reference to the tree from
	// which the dragged items originate.
	
	public function rightTreeItemDropped( $e, $tree, $rows, $targetRow, $orientation ) {
		foreach ( $rows as $row ) {
			// Check if the row originates from our own dataset
			if( $tree === $this->rightTree ) {
				// If so, we're reordering and not copying so remove the row from its original position.
				$newRow = $row->remove();
			}
			else {
				// Dropped row is foreign, so we copy the data into a new row and leave the original alone
				$newRow = $this->rightData->createRow( array( 'name' => $row->name, 'description' => $row->description ) );
			}
			
			if( $orientation == -1 ) {
				// Dropped above target
				$this->rightData->insertRowBefore( $newRow, $targetRow );
			}
			else {
				// Dropped below target
				$this->rightData->insertRowAfter( $newRow, $targetRow );
			}
		}
		
		// Check if the placeholder was used, if so remove the placeholder because now we have actual
		// rows in the tree
		
		if( $targetRow === $this->placeHolderRow ) $targetRow->remove();
	}
}

