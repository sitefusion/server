<?php

class Utility {

    /**
     * Pauses all execution by making STD block and requesting input
     */
    static public function Pause() {
        $info = stream_get_meta_data(STDIN);
        $oldState = $info['blocked'];
        stream_set_blocking(STDIN, TRUE);
        fgets(STDIN);
        stream_set_blocking(STDIN, $oldState);
    }

    static public function ValidateIPAddress($ip) {

        $noIP = false;
        if (substr_count($ip, ".") == 3) {
            foreach (explode(".", $ip) as $k => $part) {
                $partInt = intval($part);
                if (!($partInt >= 0 && $partInt <= 255)) {
                    $noIP = true;
                    break;
                }
            }
        } else {
            $noIP = true;
        }
        
        return !$noIP;
    }

    static public function ParseXHProfOutput($info, $sortMethod = 'wt') {
        if (!($sortMethod == 'ct' || $sortMethod == 'wt')) {
            trigger_error("SortMethod either CT | WT");
            return array();
        }

        $new = array();
        foreach ($info as $k => $v) {
            if ($k == "main()") {
                $target = "main()";
            } else {
                list(, $target) = @explode("==>", $k);
            }

            if (!isset($new[$target]))
                $new[$target] = array("CallTime" => 0, "WaitTime" => 0);

            $new[$target]["CallTime"] += $v["ct"];
            $new[$target]["WaitTime"] += $v["wt"];
        }

        if ($sortMethod == 'wt')
            $sortMethod = 'WaitTime';
        else
            $sortMethod = 'CallTime';

        uasort($new, function($a, $b) use ($sortMethod) {
                if ($a[$sortMethod] == $b[$sortMethod])
                    return 0;
                return $a[$sortMethod] < $b[$sortMethod] ? 1 : -1;
            });

        return $new;
    }

    /**
     * Finds out what the name of the local machine is.
     * @return string
     */
    public static function ResolveLocalMachineName() {
        $ips = explode(" ", `hostname -I`);
        foreach ($ips as $ip) {
            $trimmed = trim($ip);
            $name = Brightfish\Daemon\ServerConfig::GetNameFromIP($trimmed);
            if ($name !== NULL) {
                return $name;
            }
        }
        return "";
    }

    static public function Array_equal($a1, $a2) {
        return @!array_diff($a1, $a2) && !array_diff($a2, $a1);
    }

    static public function WritePidFile($name) {
        if (!file_put_contents("/var/run/$name.pid", getmypid())) {
            die("Failed to write PID file. Exiting...");
        }
    }

    static public function ClearDir($dir, $level = 0) {
        if ($dir && $dir != "" && $dir != "/" && strpos($dir, \Brightfish\Daemon\Daemon::$TempDir) !== FALSE) {
            if (!file_exists($dir)) {
                \Utility::Log("ClearDir:: $dir does not exist.", 0, 31);
                return;
            }
            $dirEntries = scandir($dir);
            foreach ($dirEntries as $dirEntry) {
                if ($dirEntry != '.' && $dirEntry != '..') {
                    if (!is_link($dir . "/" . $dirEntry) && is_dir($dir . "/" . $dirEntry)) {
                        \Utility::ClearDir($dir . "/" . $dirEntry, $level + 1);
                    } else {
                        unlink($dir . "/" . $dirEntry);
                    }
                }
            }
            if ($level != 0) {
                rmdir($dir);
            }
        } else {
            throw new \Exception("dir is '$dir' - probably config failed to load, exiting (td = '" . \Brightfish\Daemon\Daemon::$TempDir . "')\n");
        }
    }

    /**
     * Get own local network IP address by using hostname -I
     * @return string IPAddress
     */
    static public function GetSelfIP() {
        return explode(' ', trim(str_replace("\n", "", shell_exec("hostname -I"))));
    }

    const DB_COLOR_LIGHTGRAY = 0;
    const DB_COLOR_BLACK = 30;
    const DB_COLOR_RED = 31;
    const DB_COLOR_GREEN = 32;
    const DB_COLOR_YELLOW = 33;
    const DB_COLOR_LIGHTBLUE = 34;
    const DB_COLOR_MAGENTA = 35;
    const DB_COLOR_BLUE = 36;
    const DB_COLOR_WHITE = 37;

    static private $TimeColor = 0;

    static public function SetTimeColor($color) {
        if ($color === 0 || ($color > 29 && $color < 38))
            self::$TimeColor = $color;
        else
            throw new \Exception("Non valid color for logging chosen!");
    }

    static public function SetLogPath($path) {
        self::$logPath = $path;
    }

    static public $logPrefix = '';

    static public function SetLogPrefix($prefix = '') {
        self::$logPrefix = $prefix;
    }

    static private $logPath = '';
    static public $printLogs = FALSE;
    static private $logPointer = NULL;

    /**
     * Adds the given $in to the log, and if running in client (debug) mode, prints it to STDOUT.
     * @param mixed $in Any variable you want printed. Arrays are visualized through print_r, objects through var_export, any other are just printed.
     * @param type $tabs Amount of 'tabs' (2 spaces) before each line
     * @param type $color An UNIX color code. (0, 30 => 37). Also defined in\Utility::DB_COLOR_*.
     */
    static public function Log($in, $tabs = 0, $color = 0) {
        if (is_array($in)) {
            $str = print_r($in, TRUE);
            $lines = explode("\n", $str);
        } else if (is_object($in)) {
            if (method_exists($in, "__toString()")) {
                $str = $in->__toString();
                $lines = explode("\n", $str);
            } else {
                $str = var_export($in, TRUE);
                $lines = explode("\n", $str);
            }
        } else {
            $lines = explode("\n", $in . '');
        }

        $d = date("d-m-Y H:i:s", time());
        if (self::$logPointer === NULL) {
            if (self::$logPath)
                self::$logPointer = fopen(self::$logPath . '/run.log', 'a');
        }

        $lineBeginColor = "\033[1;" . self::$TimeColor . "m{$d}\033[0m | ";
        $lineBegin = "$d | ";
        if (self::$logPrefix != '') {
            $lineBegin .= str_pad(self::$logPrefix, 6, " ") . " | ";
            $lineBeginColor .= str_pad(self::$logPrefix, 6, " ") . " | ";
        }
        $lineBegin .= str_repeat("  ", $tabs);
        $lineBeginColor .= str_repeat("  ", $tabs);

        foreach ($lines as $line) {

            if (self::$printLogs) {
                if ($color == 0) {
                    echo $lineBeginColor . $line . "\n";
                } else {
                    echo "{$lineBeginColor}\033[1;{$color}m{$line}\033[0m\r\n";
                }
            }
            $output = "{$lineBegin}{$line}\n";
            if (self::$logPointer)
                fputs(self::$logPointer, $output, strlen($output));
            else
                echo "{$lineBegin}{$line}\n";
        }
    }

    static public function LogError($errorString) {
		if($errorString instanceof \Exception) {
			$ex = $errorString;
			$format = "Exception thrown at file %s at line %d with code %d: %s\n%s";
			$errorString = sprintf($format, $ex->getFile(), $ex->getLine(), $ex->getCode(), $ex->getMessage(), $ex->getTraceAsString());
			while($prev = $ex->getPrevious()) {
				$errorString .= "\nPrevious: " . sprintf($format, $prev->getFile(), $prev->getLine(), $prev->getMessage(), $prev->getTraceAsString());
			}
		}

        $normalLog = self::$logPointer;
        self::$logPointer = fopen(self::$logPath . '/error.log', 'a');
        self::Log($errorString);
        fclose(self::$logPointer);
        self::$logPointer = $normalLog;
    }

    static public function DumpError($variable, $level, $varName, $obProps, $knownObjects) {
        $normalLog = self::$logPointer;
        self::$logPointer = fopen(self::$logPath . '/error.log', 'a');
        self::SmartDump($variable, $level, $varName, $obProps, $knownObjects);
        fclose(self::$logPointer);
        self::$logPointer = $normalLog;
    }

    private static $files = array();

    static public function OpenNewLogFile($name) {
        $p = fopen(self::$logPath . "/$name.log", "a+");
        if ($p !== FALSE) {
            self::$files[$name] = $p;
            return TRUE;
        }
        return FALSE;
    }

    static public function CloseLogFile($name) {
        if (array_key_exists($name, self::$files)) {
            fclose(self::$files[$name]);
            unset(self::$files[$name]);
            return TRUE;
        }
        return FALSE;
    }

    static public function LogToFile($name, $text) {
        if (array_key_exists($name, self::$files)) {
            if (strrpos($text, "\n") !== 0)
                $text .= "\n";

            return (fwrite(self::$files[$name], date("d-m-Y H:i:s") . " | " . $text) !== FALSE);
        }
        return FALSE;
    }

    static public function ColorString($string, $color) {
        if (self::$printLogs)
            return "\033[1;{$color}m{$string}\033[0m";
        else
            return $string;
    }

    static private function _SmartDump($in, $level, $varName, $obProps, &$knownObjects, $return = FALSE, &$string = '') {
        $type = gettype($in);

        switch ($type) {
            case "object":
                if (!in_array($in, $knownObjects, TRUE)) {
                    $knownObjects[] = $in;

					if ($in instanceof DOMDocument) {
						$line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("DomDocument", self::DB_COLOR_GREEN) . ") " . get_class($in) . " { \n";
						$in->preserveWhiteSpace = true;
                        $in->formatOutput = true;

						$xml = $in->saveXml($in);
                        $xml = str_repeat("  ", $level) . implode("\n" . str_repeat("  ", $level), explode("\n", $xml));
                        $line .= $xml;

                        $line .= "\n}";
                        if ($return)
                            $string .= str_repeat("  ", $level) . $line . "\n";
                        else
                            self::Log($line, $level);

					} else if ($in instanceof DOMNode) {
                        $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("DomNode", self::DB_COLOR_GREEN) . ") " . get_class($in) . " { \n";

                        $in->ownerDocument->preserveWhiteSpace = true;
                        $in->ownerDocument->formatOutput = true;

                        $xml = $in->ownerDocument->saveXml($in);
                        $xml = str_repeat("  ", $level) . implode("\n" . str_repeat("  ", $level), explode("\n", $xml));
                        $line .= $xml;

                        $line .= "\n}";
                        if ($return)
                            $string .= str_repeat("  ", $level) . $line . "\n";
                        else
                            self::Log($line, $level);
                    }else {

                        $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("Object", self::DB_COLOR_GREEN) . ") " . get_class($in) . " {";
                        if ($return)
                            $string .= str_repeat("  ", $level) . $line . "\n";
                        else
                            self::Log($line, $level, 0);

                        $reflector = new ReflectionObject($in);
                        $static = $reflector->getStaticProperties();
                        $props = $reflector->getProperties();
                        foreach ($props as $refProp) {
                            $refProp->setAccessible(TRUE);
                            $name = $refProp->getName();
                            $fullName = get_class($in) . '.' . $name;

                            // Fix for namespacing (so you don't have to include the entire namespace path in the given object filter)
                            $parts = explode("\\", $fullName);
                            $fullName = end($parts);

                            if (!$obProps || array_search($fullName, $obProps, TRUE) !== FALSE) {
                                if (array_key_exists($name, $static)) {
                                    $string .= self::_SmartDump($refProp->getValue($in), $level + 2, self::ColorString(ucfirst($name), self::DB_COLOR_YELLOW), $obProps, $knownObjects, $return);
                                } else {
                                    $string .= self::_SmartDump($refProp->getValue($in), $level + 2, $name, $obProps, $knownObjects, $return);
                                }
                            }
                        }

                        if ($return)
                            $string .= str_repeat("  ", $level) . "}\n";
                        else
                            self::Log("}", $level);
                    }
                } else {
                    $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("Object", self::DB_COLOR_GREEN) . ") " . get_class($in) . " -- {" . self::ColorString("recursive reference", 31) . "}";
                    if ($return)
                        $string .= str_repeat("  ", $level) . $line . "\n";
                    else
                        self::Log($line, $level);
                }
                break;
            case "array":

                if (count($in) > 0) {
                    $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("Array", self::DB_COLOR_GREEN) . ") <L=" . count($in) . "> [";
                    if ($return)
                        $string .= str_repeat("  ", $level) . $line . "\n";
                    else
                        self::Log($line, $level);

                    foreach ($in as $key => $value) {
                        $string .= self::_SmartDump($value, $level + 2, "[$key]", $obProps, $knownObjects, $return);
                    }

                    if ($return)
                        $string .= str_repeat("  ", $level) . "]\n";
                    else
                        self::Log("]", $level);
                } else {
                    $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("Array", self::DB_COLOR_GREEN) . ") <L=0> [  ]";
                    if ($return)
                        $string .= str_repeat("  ", $level) . $line . "\n";
                    else
                        self::Log($line, $level);
                }

                break;
            case "boolean":
                $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("bool", self::DB_COLOR_BLUE) . ") " . ($in ? "TRUE" : "FALSE");
                if ($return)
                    $string .= str_repeat("  ", $level) . $line . "\n";
                else
                    self::Log($line, $level);
                break;
            case "string":

                // TODO: Fix so no binary characters are printed/returned


                $isAscii = mb_detect_encoding($in, 'ASCII', true);
				$isAscii = TRUE;
                if ($isAscii) {
                    $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("string", self::DB_COLOR_BLUE) . " L=" . strlen($in) . ") " . $in;
                } else {
                    $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("string", self::DB_COLOR_BLUE) . " (" . self::ColorString("hex", self::DB_COLOR_RED) . ") L=" . strlen($in) . ") " . bin2hex($in);
                }

                if ($return)
                    $string .= str_repeat("  ", $level) . $line . "\n";
                else
                    self::Log($line, $level);

                break;
            case "resource":
                $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("resource", self::DB_COLOR_YELLOW) . ") " . $in;

                if ($return)
                    $string .= str_repeat("  ", $level) . $line . "\n";
                else
                    self::Log($line, $level);
                break;
            case "NULL":
                $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString("NULL", self::DB_COLOR_RED) . ") " . $in;

                if ($return)
                    $string .= str_repeat("  ", $level) . $line . "\n";
                else
                    self::Log($line, $level);

                break;
            default:
                $line = ($varName !== NULL ? $varName : "") . " => (" . self::ColorString($type, self::DB_COLOR_BLUE) . ") " . $in;

                if ($return)
                    $string .= str_repeat("  ", $level) . $line . "\n";
                else
                    self::Log($line, $level);

                break;
        }
        return $string;
    }

    /**
     * DEBUG ONLY FUNCTION!
     * Visualize any variable
     * Calls _dump() if the method exists on a object
     *  Sets object properties to public during runtime using Reflection
     * @param mixed $in The variable to you want to print
     * @param integer $level Amount of tabs (for recursion)
     * @param string $varName The name of the variable
     * @param array $obProps Variable names to print (of objects) (array of strings with {Class}.{Property})
     * @param boolean $return Whether to return the log as a string instead of putting it to STDOUT
     * @return string If $return is TRUE,
     */
    static public function SmartDump($in, $level = 0, $varName = NULL, $obProps = NULL, $return = FALSE) {
        $x = array();
        $s = '';

		if(is_string($level)) {
			$varName = $level;
			$level = 0;
		}

        return self::_SmartDump($in, $level, $varName, $obProps, $x, $return, $s);
    }

}