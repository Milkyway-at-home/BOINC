<?php

// script for submitting batches of LAMMPS jobs
//
// When a batch is submitted, this script runs the first job
// until 1 time step is completed;
// this verifies that the input files are OK and gives us
// an estimate of the FLOPS needed for the batch.
//
// These test executions are done in the directory
// project/lammps_test/USERID.
// We assume that lammps_test exists and contains the LAMMPS executable

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/util.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/sandbox.inc");

// test a LAMMPS job
//
// the current directory must contain
//      structure_file
//      lammps_script
//      cmd_variables
//      pot_files (zipped potential files)
//
// output: success flag, CPU time per step, est. disk usage per job
//
function lammps_est() {
    $avg_cpu = 0;
    $disk_space = 0;
    $test_result = 0;
    $descs = array();
    $pipes = array();
    $options = file("cmd_variables");
    $cmd = "../lmp_linux ".$options[0];
    echo $cmd;
    $p = proc_open("$cmd", $descs, $pipes);
    system("unzip pot.zip");
    while (1) {
        if (file_exists("log.1")) {
            $avg_cpu = calc_step_cpu("log.1");
            if ($avg_cpu != 0) {
                //echo "avg_cpu is ".$avg_cpu."\n";
                proc_terminate($p);
                $test_result = 1;
                break;
            }
        }
        //echo "sleeping\n";
        sleep(1);
    }

    return array($test_result, $avg_cpu, $disk_space);
}

function calc_step_cpu($filename) {
    $fd = fopen("$filename", "r");
    $start_line = "Step CPU flow_com avoo";
    $start = 0;
    $start_step = 1;
    $cur_step = 1;
    $avg_cpu = 0;
    if (!$fd) {
        echo "fail to open file log.1";
        exit(-1);
    }
    while (!feof($fd)) {
        $line = fgets($fd,4096);
        if (strlen($line) > strlen($start_line)
            && substr_compare($line, $start_line, 0, strlen($start_line)) == 0
        ) {
            $start = 1;
            continue;
        }
        if (!$start) continue;
        $arr = preg_split("/\s+/", $line);
        if (count($arr)!=6 || !is_numeric($arr[1])) {
            continue;
        }
        $step = (int)$arr[1];
        $cpu = (float)$arr[2];
        if ($cpu==0) {
            $start_step = $step;
        } else {
            $cur_step = $step;
            if ($cur_step-$start_step>=5) {
                $avg_cpu = $cpu/($cur_step-$start_step);
                //echo "avg_cpu is ".$avg_cpu;
                break;
            }
        }
    }
    return $avg_cpu;
}

function area_select() {
    return "
        <select name=area>
        <option value=\"Air filtration\">Air filtration</option>
        <option value=\"Water filtration\">Water filtration</option>
        <option value=\"Ultra-low friction\">Ultra-low friction</option>
        </select>
    ";
}

function show_submit_form($user) {
    page_head("Submit LAMMPS jobs");
    echo "
        <form action=lammps.php>
        <input type=hidden name=action value=prepare>
    ";
    start_table();
    row2("Structure file", sandbox_file_select($user, "structure_file"));
    row2("Script file", sandbox_file_select($user, "command_file"));
    row2("Command-line file<br><span class=note>List of command lines, one per job</span>", sandbox_file_select($user, "cmdline_file"));
    row2("Zipped potential files", sandbox_file_select($user, "pot_files"));
    row2("Area", area_select());
    row2("", "<input type=submit value=Prepare>");
    end_table();
    echo "</form>
        <p>
        <a href=sandbox.php>File sandbox</a>
    ";
    
    page_tail();
}

// verify that an input file exists in sandbox, and return its physical path
//
function get_file_path($user, $name) {
    $fname = get_str($name);

    // verify that the files exist in sandbox
    //
    $sbdir = sandbox_dir($user);
    list($error, $size, $md5) = sandbox_parse_link_file("$sbdir/$fname");
    if ($error) error_page("no $name file");

    return sandbox_physical_path($user, $md5);
}

function project_flops() {
    $x = BoincUser::sum("expavg_credit");
    if ($x == 0) $x = 200;
    $y = 1e9*$x/200;
    return $y;
}

// Estimate how long a batch will take.
// Let N = # jobs, M = # hosts
// If N < M we can start all the jobs more or less now,
// so let T = F/H, where H is the median FLOPS of the hosts.
// If N > M we'll have to do the jobs in stages,
// so let T = ceil(N/M)*F/H.
//
// Note: these are both extremely optimistic estimates
//
function estimated_makespan($njobs, $flops_per_job) {
    $nhosts = BoincHost::count("expavg_credit > 1");
    if ($nhosts < 10) {
        $median_flops = 2e9;
    } else {
        $n = $nhosts/2;
        $hs = BoincHost::enum("expavg_credit>1 order by p_fpops limit 1");
        $h = $hs[0];
        $median_flops = $h->p_fpops;
    }

    if ($njobs < $nhosts) {
        return $flops_per_job/$median_flops;
    } else {
        $k = (int)(($njobs+$nhosts-1)/$nhosts);
        return $k*$flops_per_job/$median_flops;
    }
}

function prepare_batch($user) {
    $structure_file_path = get_file_path($user, 'structure_file');
    $command_file_path = get_file_path($user, 'command_file');
    $cmdline_file_path = get_file_path($user, 'cmdline_file');
    $pot_files_path = get_file_path($user, 'pot_files');

    $info = null;
    $info->structure_file_path = $structure_file_path;
    $info->command_file_path = $command_file_path;
    $info->cmdline_file_path = $cmdline_file_path;
    $info->pot_files_path = $pot_files_path;
    $info->area = get_str("area");

    // get the directory in which to run the test,
    // clear it out,
    // and set up links to the input files
    //
    $test_dir = "../../lammps_test/$user->id";
    //echo "test_dir is ".$test_dir;
    if (!is_dir($test_dir)) {
        mkdir($test_dir);
    }
    if (!chdir($test_dir)) {
        error_page("Can't chdir");
    }
    system("rm *");
    symlink($structure_file_path, "structure_file");
    symlink($command_file_path, "lammps_script");
    symlink($cmdline_file_path, "cmd_variables");
    symlink($pot_files_path, "pot_files");
    list($error, $est_cpu_time, $disk) = lammps_est();

    if ($error==0) {
        error_page("LAMMPS test failed");
    }


    system("rm *");
    $info->rsc_fpops_est = $est_cpu_time * 5e9;

    $info->rsc_disk_bound = 1e8;

    $tmpfile = tempnam("/tmp", "lammps_");
    file_put_contents($tmpfile, serialize($info));

    // get the # of jobs
    //
    $njobs = count(file($cmdline_file_path));
    $secs_est = estimated_makespan($njobs, $info->rsc_fpops_est);
    $hrs_est = number_format($secs_est/3600, 1);
    $client_mb = $info->rsc_disk_bound/1e6;
    $server_mb = $njobs*$client_mb;

    page_head("Batch prepared");
    echo "
        Your batch has $njobs jobs.
        <p>
        Estimated time to completion: $hrs_est hours.
        <p>
        Estimated client disk usage: $client_mb MB
        <p>
        Estimated server disk usage: $server_mb MB
        <p>
    ";
    show_button("lammps.php?action=submit&tmpfile=$tmpfile", "Submit Batch");
    page_tail();
}

function submit_job($app, $batch_id, $info, $cmdline, $i) {
    $cmd = "cd ../..; ./bin/create_work --appname $app->name --batch $batch_id --rsc_fpops_est $info->rsc_fpops_est";
    if ($cmdline) {
        $cmd .= " --command_line \"$cmdline\"";
    }
    $cmd .= " --wu_name batch_".$batch_id."_".$i;
    $cmd .= " ".basename($info->structure_file_path);
    $cmd .= " ".basename($info->command_file_path);
    $cmd .= " ".basename($info->pot_files_path);
    echo "<br> $cmd\n"; 

    $ret = system($cmd);
    if ($ret === FALSE) {
        error_page("can't create job");
    }
}

function submit_batch($user, $app) {
    $tmpfile = get_str('tmpfile');
    $x = file_get_contents("$tmpfile");
    $info = unserialize($x);

    $cmdlines = file($info->cmdline_file_path);
    $njobs = count($cmdlines);

    $now = time();
    $batch_name = $info->area;

    $batch_id = BoincBatch::insert(
        "(user_id, create_time, njobs, name, app_id) values ($user->id, $now, $njobs, '$batch_name', $app->id)"
    );
//    $batch_id=99;

    $i = 0;
    foreach ($cmdlines as $cmdline) {
        submit_job($app, $batch_id, $info, $cmdline, $i);
        $i++;
    }
}

$user = get_logged_in_user();
$user_submit = BoincUserSubmit::lookup_userid($user->id);
if (!$user_submit) error_page("no submit access");
$app = BoincApp::lookup("name='lammps'");
if (!$app) error_page("no lammps app");

if (!$user_submit->submit_all) {
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id");
    if (!$usa) {
        error_page("no submit access");
    }
}

$action = get_str('action', true);
switch ($action) {
case '': show_submit_form($user); break;
case 'prepare': prepare_batch($user); break;
case 'submit': submit_batch($user, $app); break;
default: error_page("no such action $action");
}
?>
