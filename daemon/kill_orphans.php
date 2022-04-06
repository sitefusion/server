#!/usr/bin/env php
<?php

require_once(__DIR__ . '/../conf/daemon.conf');
require_once(__DIR__ . '/../daemon/error.php.inc');
require_once(__DIR__ . '/../daemon/daemon.php.inc');

$silent = in_array('-q', $argv) || in_array('--quiet', $argv);
$dryRun = in_array('-d', $argv) || in_array('--dry-run', $argv);

if (!isset($SFCONFIG['sitefusionPath'])) {
    echo 'Unable to get "sitefusionPath" from config' . PHP_EOL;
    exit(1);
}

function parseStatus($strStatus) {

    preg_match_all('/^(?<key>[^:]+):\s*(?<value>.+)$/m', $strStatus, $matches, PREG_OFFSET_CAPTURE);

    $data = [];
    for ($i = 0; $i < count($matches['key']); $i++) {

        $key = $matches['key'][$i][0];
        $value = $matches['value'][$i][0];

        $data[$key] = $value;

    }

    return $data;
}

$execPath = $SFCONFIG['sitefusionPath'] . '/daemon/start.php';

$files = glob('/proc/*/');
foreach ($files as $filePath) {

    $statusPath = $filePath . '/status';
    $cmdlinePath = $filePath . '/cmdline';

    if (!file_exists($cmdlinePath)) {
        continue;
    }

    $cmdlineContent = @file_get_contents($cmdlinePath);
    $statusContent = @file_get_contents($statusPath);

    if ($cmdlineContent === FALSE || $statusContent === FALSE) {
        continue; // Fix for the async-ness of the procfs
    }

    $status = parseStatus($statusContent);

    $cmdlineParts = explode("\0", $cmdlineContent);
    if (isset($cmdlineParts[1]) && $cmdlineParts[1] === $execPath) {

        if ($status['PPid'] === '1') {

            if (!$silent) {
                echo 'Got orphan ' . $status['Pid'] . PHP_EOL;
            }

            if (!$dryRun) {

                posix_kill($status['Pid'], 9);

            }

        }

    }

}
