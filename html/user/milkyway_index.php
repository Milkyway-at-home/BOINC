<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/news.inc");
require_once("../inc/cache.inc");
require_once("../inc/uotd.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/translation.inc");
require_once("../inc/text_transform.inc");
require_once("../project/project.inc");

function show_nav() {
    $config = get_config();
    $master_url = parse_config($config, "<master_url>");
    echo "<div id=\"mainnav\">
	<img src=\"img/mw.png\" alt=\"milkyway@home\" width=\"597\" height=\"151\" align=\"top\" />
        <h2>About ".PROJECT."</h2>

	<p>
	Milkyway@Home uses the BOINC platform to harness volunteered computing resources, creating a highly accurate three dimensional model of the Milky Way galaxy using data gathered by the <a href='http://www.sdss.org/'>Sloan Digital Sky Survey</a>.  This project enables research in both astroinformatics and computer science.
	</p>

	<p>
	In computer science, the project is investigating different optimization methods which are resilient to the fault-prone, heterogeneous and asynchronous nature of Internet computing; such as evolutionary and genetic algorithms, as well as asynchronous newton  methods.  While in astroinformatics, Milkyway@Home is generating highly accurate three dimensional models of the Sagittarius stream, which provides knowledge about how the Milky Way galaxy was formed and how tidal tails are created when galaxies merge.
	</p>

	<p>
	Milkyway@Home is a joint effort between <a href='http://www.rpi.edu'>Rensselaer   Polytechnic Institute</a>'s departments of <a href='http://www.cs.rpi.edu'>Computer Science</a> and <a href='http://www.rpi.edu/dept/phys/'>Physics, Applied Physics and   Astronomy</a>.  Feel free to contact us via our forums, or email astro [at] cs [dot] rpi [dot] edu.
	</p>

        <h2>Join ".PROJECT."</h2>
        <ul>
        <li><a href=\"info.php\">".tra("Read our rules and policies")."</a>
        <li> This project uses BOINC.
            If you're already running BOINC, select Attach to Project.
            If not, <a target=\"_new\" href=\"http://boinc.berkeley.edu/download.php\">download BOINC</a>.
        <li> When prompted, enter <br><b>".$master_url."</b>
        <li> If you're running a command-line or pre-5.0 version of BOINC,
            <a href=\"create_account_form.php\">create an account</a> first.
        <li> If you have any problems,
            <a target=\"_new\" href=\"http://boinc.berkeley.edu/help.php\">get help here</a>.
        </ul>


        </ul>
        <h2>".tra("Community")."</h2>
        <ul>
	<li><a href=\"http://www.dudleyobservatory.org/MilkyWayAtHome/MilkyWayAtHome2.html\"> Donate to MilkyWay@Home via the Dudley Observatory</a></li>
        <li><a href=\"forum_index.php\">".tra("Message boards")."</a>
	<li><a href=\"server_status.php\">".tra("Server Status")."</a>
        <li><a href=\"profile_menu.php\">".tra("Profiles")."</a>
        <li><a href=\"user_search.php\">User search</a>
        <li><a href=\"stats.php\">Statistics</a>
        <li><a href=language_select.php>Languages</a>
        </ul>

	<h2>Science</h2>
	<ul>
	<li><a href=\"plots.php\">".tra("Search Progress Plots")."</a>
	<li><a href=\"science.php\">".tra("Astrophysics Overview")."</a>
	<li><a href=\"information.php\">".tra("Publications, Public Talks and Funding")."</a>
	<li><a href=\"donations.php\">".tra("Donations")."</a>
	</ul>

 <h2>Returning participants</h2>
        <ul>
        <li><a href=\"home.php\">Your account</a> - view stats, modify preferences
        <li><a href=\"team.php\">Teams</a> - create or join a team
        <li><a href=\"cert1.php\">Certificate</a>
        <li> <a href=\"apps.php\">".tra("Applications")."</a>
	</ul>

        <h2>People</h2>
        Milkyway@Home is currently run by:
	<ul>
	<li> <a href='http://www.cs.rpi.edu/~deselt/'>Travis Desell</a>, Assistant Professor of Computer Science, University of North Dakota</li>
	<li> <a href='http://www.cs.rpi.edu/~magdon/'>Malik Magdon-Ismail</a>, Associate Professor of Computer Science </li>
	<li> <a href='http://www.rpi.edu/~newbeh/'>Heidi Newberg</a>, Associate Professor of Physics, Applied Physics, and Astronomy </li>
	<li> Matthew Newby, Graduate Research Assistant in Physics, Applied Physics and Astronomy </li>
	<li> <a href='http://www.cs.rpi.edu/~szymansk/index.php'>Boleslaw Szymanski</a>, Claire and Roland Schmitt Distinguished Professor of Computer Science </li>
	<li> <a href='http://www.cs.rpi.edu/~cvarela'>Carlos Varela</a>, Associate Professor of Computer Science </li>
	<li> Benjamin Willet, Postdoctoral Researcher in Physics, Applied Physics and Astronomy </li>
	<li> Matt Arsenault, Graduate Research Assistant in Physics</li>
	<li> Colin Rice, Undergraduate Research Assistant in Physics</li>
	</ul>

	With previous support from:
	<ul>
	  <li> Nathan Cole, Graduate Research Assistant in Physics, Applied Physics   and Astronomy </li>
	  <li> Joe Doran, Undergraduate Research Assistant in Computer Science </li>
	  <li> Dave Przybylo, Undergraduate Research Assistant in Computer Science </li>
	<li> Shane Reilly, Undergraduate Research Assistant in Computer Science </li>
	<li> <a href='http://brianchitester.com'>Brian Chitester</a>, Undergraduate Research Assistant in Computer Science</li>
	<li> John Vickers, Undergraduate Research Assistant in Physics, Applied Physics and Astronomy </li>
	<li> Anthony Waters, Graduate Research Assistant in Computer Science </li>
	</ul>
	</div>
    ";
}

$caching = false;

if ($caching) {
    start_cache(INDEX_PAGE_TTL);
}

$stopped = web_stopped();
$rssname = PROJECT . " RSS 2.0" ;
$rsslink = URL_BASE . "rss_main.php";

header("Content-type: text/html; charset=utf-8");

echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">";


echo "<html>
    <head>
    <title>".PROJECT."</title>
	<link rel=\"stylesheet\" type=\"text/css\" href=\"main.css\" media=\"all\" />
    <link rel=\"stylesheet\" type=\"text/css\" href=\"".STYLESHEET."\">
    <link rel=\"alternate\" type=\"application/rss+xml\" title=\"".$rssname."\" href=\"".$rsslink."\">
";

include 'schedulers.txt';

echo "
    </head><body>
    <span class=page_title>".PROJECT."</span>
    <table cellpadding=\"8\" cellspacing=\"4\">
    <tr><td rowspan=\"2\" valign=\"top\" width=\"40%\">
";

if ($stopped) {
    echo "
        <b>".PROJECT." is temporarily shut down for maintenance.
        Please try again later</b>.
    ";
} else {
    db_init();
    show_nav();
}

echo "
    <p>
    <a href=\"http://boinc.berkeley.edu/\"><img align=\"middle\" border=\"0\" src=\"img/pb_boinc.gif\" alt=\"Powered by BOINC\"></a>
    </p>
    </td>
";

if (!$stopped) {
    $profile = get_current_uotd();
    if ($profile) {
        echo "
            <td class=\"uotd\">
            <h2>".tra("User of the day")."</h2>
        ";
        show_uotd($profile);
        echo "</td></tr>\n";
    }
}

echo "
    <tr><td class=\"news\">
    <h2>News</h2>
    <p>
";
//include("motd.php");
show_news(0, 10);
echo "
    </td>
    </tr></table>
";


if ($caching) {
    page_tail_main(true);
    end_cache(INDEX_PAGE_TTL);
} else {
    page_tail_main();
}

?>
