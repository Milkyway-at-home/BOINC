<?php

// RSS feed for per-user notifications

require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");
require_once("../inc/pm.inc");
require_once("../inc/friend.inc");
require_once("../inc/notify.inc");
require_once("../project/project.inc");

$userid = get_int('userid');
$auth = get_str('auth');

$user = BoincUser::lookup_id($userid);
if (!$user) xml_error();
if (notify_rss_auth($user) != $auth) xml_error();

$notifies = BoincNotify::enum("userid = $userid order by create_time desc");

if (count($notifies)) {
    $last_mod_time = $notifies[0]->create_time;
} else {
    $last_mod_time = time();
}
$create_date  = gmdate('D, d M Y H:i:s', $last_mod_time) . ' GMT'; 

header("Expires: ".gmdate('D, d M Y H:i:s', time())." GMT");
header("Last-Modified: ".$create_date);
header("Content-Type: application/xml");

$description = "Community notifications";
$channel_image = URL_BASE."rss_image.gif";
$language = "en-us";
echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
    <rss version=\"2.0\">
    <channel>
    <title>".PROJECT."</title>
    <link>".URL_BASE."</link>
    <description>".$description."</description>
    <copyright>".COPYRIGHT_HOLDER."</copyright>
    <lastBuildDate>".$create_date."</lastBuildDate>
    <language>".$language."</language>
    <image>
        <url>".$channel_image."</url>
        <title>".PROJECT."</title>
        <link>".URL_BASE."</link>
    </image>
";

foreach ($notifies as $notify) {
    switch ($notify->type) {
    case NOTIFY_FRIEND_REQ:
        friend_req_rss($notify, $title, $msg, $url);
        break;
    case NOTIFY_FRIEND_ACCEPT:
        friend_accept_rss($notify, $title, $msg, $url);
        break;
    case NOTIFY_PM:
        pm_rss($notify, $title, $msg, $url);
        break;
    case NOTIFY_SUBSCRIBED_POST:
        subscribe_rss($notify, $title, $msg, $url);
        break;
    }

    $news_date=gmdate('D, d M Y H:i:s',$notify->create_time) . ' GMT';
    $unique_url=URL_BASE."home.php";
    echo "<item>
        <title>$title</title>
        <link>".htmlentities($url)."</link>
        <description><![CDATA[$msg]]></description>
        <pubDate>$news_date</pubDate>
        </item>
    ";
}

echo "
    </channel>
    </rss>
";

?>
