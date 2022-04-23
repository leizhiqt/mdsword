./module/server -ntp 192.168.1.70 > s.log 2>&1 & 

./module/server -ntp 192.168.1.70 > /dev/null 2>&1 & 

nohup ./module/server -ntp 192.168.1.70 > /dev/null 2>&1 & 

# jobs      //查看任务，返回任务编号n和进程号
# bg  %n   //将编号为n的任务转后台运行
# fg  %n   //将编号为n的任务转前台运行
# ctrl+z    //挂起当前任务
# ctrl+c    //结束当前任务
