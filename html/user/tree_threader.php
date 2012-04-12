<?php

// Handler for TreeThreader remote job submission.
//
// Assumptions:
// - there is a file "tree_threader_templates_files" in the project root
//   containing (one per line) the names of files containing
//   gzipped collections of template files
// - These files are in the download hierarchy.

require_once("../inc/boinc_db.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/xml.inc");
require_once("../inc/dir_hier.inc");
require_once("../inc/result.inc");
require_once("../inc/submit_util.inc");

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

$app_name = "uppercase";

function error($s) {
    echo "<error>\n<message>$s</message>\n</error>\n";
    exit;
}

function handle_submit($r, $user, $app) {
	// read the list of template filenames
	//
	$files = file("../../tree_threader_template_files");
	if ($files === false) error("no templates file");
	$njobs = sizeof($files);
	$now = time();
    $batch_id = BoincBatch::insert(
		"(user_id, create_time, njobs, name, app_id, state) values ($user->id, $now, $njobs, 'test batch', $app->id, ".BATCH_STATE_IN_PROGRESS.")"
    );
	foreach ($files as $file) {
		$cmd = "cd ../..; ./bin/create_work --appname tree_thread --batch $batch_id $file";
		$ret = system($cmd);
		if ($ret === false) {
			error("can't create job");
		}
	}
	echo "<batchid>$batchid</batchid>\n";
}

// Enumerate all the successfully completed WUs for this batch.
// Combine their output files into a zip file in /tmp,
// make a symbolic link to this from /download,
// and return the resulting URL
//
function handle_get_output($r, $batch) {
	$wus = BoincWorkUnit::enum("batchid=$batch->id");
	$outdir = "/tmp/tree_threader_".$batch->id;
	foreach ($wus as $wu) {
		if (!$wu->canonical_resultid) continue;
		$result = BoincResult::lookup_id($wu->canonical_resultid);
		if (!$result) continue;
		$paths = get_outfile_paths($result);
		if (sizeof($paths) < 1) continue;

		// there's only one output file
		//
		$path = $paths[0];

		// unzip it into a directory in /tmp
		//
		$dir = tmpdir();
		$cmd = "cd $dir; unzip -r $path";
		$ret = system($cmd);
		if ($ret === false) {
			error("can't unzip output file");
		}
		$cmd = "cp $dir/* $outdir";
		$ret = system($cmd);
		if ($ret === false) {
			error("can't copy output files");
		}

	}
	$cmd = "cd /tmp ; zip -r $outdir $outdir";
	$ret = system($cmd);
	if ($ret === false) {
		error("can't zip output files");
	}
	$fname = "tree_threader_".$batch_id.".zip";
	symlink($outdir, "../../download/$fname");
	echo "<url>$fname</url>";
}

xml_header();

if (0) {
$r = simplexml_load_string($_POST['request']);
} else {
$x = <<<EOF
<tt_request>
    <action>submit</action>
    <sequence><![CDATA[
#!/usr/bin/env python

'''
Add platform and application records to the BOINC database.
Reads info from "project.xml", e.g.:

<boinc>
    <platform>
        <name>i686-pc-linux-gnu</name>
        <user_friendly_name>Linux/x86</user_friendly_name>
    </platform>
    <app>
        <name>astropulse</name>
        <user_friendly_name>AstroPulse</user_friendly_name>
    </app>
</boinc>

See http://boinc.berkeley.edu/tool_xadd.php
'''

import boinc_path_config
from Boinc import database, db_mid, projectxml

database.connect()
projectxml.default_project().commit_all()

]]></sequence>
    <auth>157f96a018b0b2f2b466e2ce3c7f54db</auth>
</tt_request>
EOF;
$r = simplexml_load_string($x);
}

if (!$r) {
    error("can't parse request message");
}

// authenticate the user
//
$auth = (string)$r->auth;
$user = BoincUser::lookup("authenticator='$auth'");
if (!$user) error("invalid authenticator");
$user_submit = BoincUserSubmit::lookup_userid($user->id);
if (!$user_submit) error("no submit access");
$app = BoincApp::lookup("name='$app_name'");
if (!$app) error("no tree_threader app");

if (!$user_submit->submit_all) {
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id");
    if (!$usa) {
        error("no submit access");
    }
}

switch ((string)$r->action) {
    case 'submit': handle_submit($r, $user, $app); break;
    case 'get_output':
		$batch_id = (int)$r->batchid;
		$batch = BoincBatch::lookup_id($batch_id);
		if (!$batch) error("no such batch");
		if ($batch->user_id != $user->id) error("not owner of batch");
		handle_get_output($r, $batch);
		break;
    default: error("bad command");
}

?>
