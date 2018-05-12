<?php
if ($_SERVER['REQUEST_METHOD'] != "POST") {
    echo "method not allowed";
    exit;
}

if ((isset($_POST['content']) && !empty($_POST['content']))
    && isset($_POST['user']) && !empty($_POST['user'])) {
    try {
        $pdo = new PDO('sqlite:db/database.sqlite');
        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING);
    } catch (PDOException $e) {
        echo 'Connection failed: ' . $e->getMessage();
        exit;
    }

    $insertQuery = 'INSERT INTO post (content,user) VALUES(?,?)';
    $sth = $pdo->prepare($insertQuery);
    $res = $sth->execute(array(
        $_POST['content'],
        $_POST['user']
    ));

    echo json_encode(array(
        'status' => $res ? 'OK' : 'ERROR',
        'msg' => $res ? '数据写入成功' : '写入失败'
    ));

} else {
    echo json_encode(array(
        'status' => 'ERROR',
        'msg' => 'content 或 user 不能为空'
    ));
}