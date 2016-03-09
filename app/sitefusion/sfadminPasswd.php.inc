<?php
/**
 * @package SiteFusionAdmin
*/


include_once 'crypt.php.inc';

// The password variable may contain a plaintext password
//$password = 'secret';
// ... or the hashed (PBKDF2) version
$hashedPassword = "fb374ff45aa18322bb30";

// In order to produce a hashed password, uncomment and run the following line with the
// desired password. Don't leave the plaintext password in there after getting the hash!
//
// var_dump(hash_password('mypassword'));
