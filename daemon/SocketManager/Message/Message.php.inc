<?php

namespace SocketManager;

class Message {

    const T_INT = 0x01;
    const T_STRING = 0x02;
    const T_FLOAT = 0x03;
    const T_BOOL = 0x04;
    const T_SERIALIZED = 0x05;
    const S_TYPE = 1;
    const S_INT = 4;
    const S_STRING_K = 4;
    const S_SERIALIZED_K = 4;
    const S_FLOAT = 4;
    const S_BOOL = 1;

    private $messages = array();

    public function __construct($p1 = FALSE) {
        if (is_array($p1)) {

            foreach ($p1 as $k => $v) {
                $this->addMessage($k, $v);
            }
        }
    }

    public function __toString() {
        $out = '';
        foreach ($this->messages as $message) {
            $out .= $message;
        }

        return pack('V', strlen($out)) . $out;
    }

    private function buildPart($type, $var) {

        switch ($type) {
            case self::T_INT:
                //echo "int\n";
                $tp = pack('Cl', $type, (int) $var);
                break;
            case self::T_STRING:
                //echo "string\n";
                // changed: 26-07-2012 from Cv to CL
                $tp = pack('CL', $type, strlen($var)) . $var;
                break;
            case self::T_FLOAT:
                //echo "float\n";
                $tp = pack('Cf', $type, (double) $var);
                break;
            case self::T_BOOL:
                //echo "bool\n";
                $tp = pack('CC', $type, (boolean) $var);
                break;
            case self::T_SERIALIZED:
                $serialized = serialize($var);
                $tp = pack('CL', $type, strlen($serialized)) . $serialized;
                break;
            default:

                \Utility::SmartDump($var);
                \Utility::Log("T= {$type} occurred in Message");

                throw new \Exception("Wrong type used in construction of a Message! key=int|string, value=int|float|string|bool");
        }

        return $tp;
    }

    public function addMessage($key, $value) {

        $kType = self::GetDataTypeShort($key);
        if ($kType != self::T_INT && $kType != self::T_STRING) {
            throw new \Exception("Key's type can only contain int and string!");
        }

        $keyPart = $this->buildPart(self::GetDataTypeShort($key), $key);
        $valuePart = $this->buildPart(self::GetDataTypeShort($value), $value);

        $this->messages[] = $keyPart . $valuePart;
    }

    private static function GetDataTypeShort($value) {
        switch (gettype($value)) {
            case 'string':
                return self::T_STRING;
            case 'integer':
                return self::T_INT;
            case 'double':
                return self::T_FLOAT;
            case 'boolean':
                return self::T_BOOL;
            case 'array':
            case 'object':
                return self::T_SERIALIZED;
            default:
                throw new \Exception("Unsupported datatype given to message: " . gettype($value));
        }
    }

    static public function Decode($input) {
        $tMessages = array();

        $newValueBlock = TRUE;

        $valueType = 0;
        $var = NULL;

        $key = NULL;
        $value = NULL;

        $pos = 0;
        $shouldHave = 0;
        $currentlyHave = 0;
        while ($pos < strlen($input)) {

            if ($pos < 4) {
                $countA = unpack('VCount', substr($input, 0, 4));
                $shouldHave = $countA['Count'];
                $pos+=4;
            }

            if ($newValueBlock) {
                $typeA = unpack('CType', substr($input, $pos, 1));
                $valueType = $typeA['Type'];
                $pos++;
                $newValueBlock = FALSE;
            }

            if ($valueType > 0) {
                switch ($valueType) {
                    case self::T_INT:
                        $val = unpack('lVal', substr($input, $pos, 4));
                        $var = (int) $val['Val'];

                        $pos += self::S_INT;
                        break;
                    case self::T_STRING:
                        $len = unpack('LLength', substr($input, $pos, 4));
                        $pos += self::S_STRING_K;

                        $var = substr($input, $pos, $len['Length']);
                        $pos += $len['Length'];
                        break;
                    case self::T_SERIALIZED:
                        $len = unpack('LLength', substr($input, $pos, 4));
                        $pos += self::S_SERIALIZED_K;

                        $var = unserialize(substr($input, $pos, $len['Length']));
                        $pos += $len['Length'];
                        break;
                    case self::T_FLOAT:
                        $val = unpack('fVal', substr($input, $pos, 4));

                        $var = (double) $val['Val'];
                        $pos += self::S_FLOAT;
                        break;
                    case self::T_BOOL:
                        $val = unpack('CVal', substr($input, $pos, 1));
                        $var = (bool) $val['Val'];
                        $pos += self::S_BOOL;
                        break;
                }
            }

            if ($key === NULL) {
                $key = $var;
                $newValueBlock = TRUE;
            } else if ($key !== NULL) {
                $value = $var;
                $newValueBlock = TRUE;
            }

            if ($key !== NULL && $value !== NULL) {
                if ($shouldHave == ++$currentlyHave) {
                    $shouldHave = 0;
                    $currentlyHave = 0;

                    $input = substr($input, $pos);
                    $pos = 0;
                }

                $tMessages[$key] = $value;

                $key = NULL;
                $value = NULL;
            }
        }

        //echo count($tMessages);
        return $tMessages;
    }

}
