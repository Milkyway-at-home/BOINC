<?php
require_once("docutil.php");
page_head("Computation credit");
echo "

<p>
Each project gives you <b>credit</b> for the computations your
computers perform for it.
BOINC's unit of credit, the <b>Cobblestone</b> <sup>1</sup>,
is 1/100 day of CPU time on a reference computer that does
<ul>
<li> 1,000 double-precision MIPS based on the Whetstone benchmark.
<li> 1,000 VAX MIPS based on the Dhrystone benchmark.
</ul>
Eventually, credit may reflect network transfer and disk storage as well
as computation.

<h2>How credit is determined</h2>

When your computer completes a result,
BOINC determines an amount of <b>claimed credit</b>
in one of two ways:
<ul>
<li>
In general, the claimed credit is the result's CPU time multiplied
by the CPU benchmarks as measured by the BOINC software.
<b>NOTE:</b> the BOINC software is not optimized for specific processors.
Its benchmark numbers may be lower than
those produced by other programs.
<li>
Some applications determine claimed credit themselves,
and report it to BOINC.
This would be the case, for example, with
applications that use graphics coprocessors or other non-CPU hardware.
</ul>

<p>
Claimed credit is reported to a project
when your computer communicates with its server.
The <b>granted credit</b> that you receive may
be different from the claimed credit,
and there may be a delay of a few hours or days
before it is granted.
This is because some BOINC projects grant credit only after
results have been <a href=intro_user.php#credit>validated</a>.



<h2>Recent Average Credit</h2>
<p>
Projects maintains two counts of granted credit:
<ul>
<li> <b>Total credit</b>:
The total number of Cobblestones performed and granted.
<li> <b>Recent average credit</b>:
The average number of Cobblestones per day granted recently.
This average decreases by a factor of two every week,
according to the algorithm given below.
</ul>

<p>
Both quantities (total and recent average)
are maintained for each user, host and team.

<p>
Each time new credit granted,
the following function is used to update the
recent average credit of the host, user and team:
<pre>",htmlspecialchars("

// Update an estimate of \"units per day\" of something (credit or CPU time).
// The estimate is exponentially averaged with a given half-life
// (i.e. if no new work is done, the average will decline by 50% in this time).
// This function can be called either with new work,
// or with zero work to decay an existing average.
//
void update_average(
    double work_start_time,       // when new work was started
                                    // (or zero if no new work)
    double work,                    // amount of new work
    double half_life,
    double& avg,                    // average work per day (in and out)
    double& avg_time                // when average was last computed
) {
    double now = dtime();

    if (avg_time) {
        double diff = now - avg_time;
        double diff_days = diff/SECONDS_PER_DAY;
        double weight = exp(-diff*M_LN2/half_life);
        avg *= weight;
        avg += (1-weight)*(work/diff_days);
    } else {
        double dd = (now - work_start_time)/SECONDS_PER_DAY;
        avg = work/dd;
    }
    avg_time = now;
}

"),"
</pre>
<hr noshade size=1>
<sup>1</sup> Named after Jeff Cobb of SETI@home
";
page_tail();
?>
