::该文件用于每次打包给测试，准则：版本号/newpackage/bin/**.exe等
::使用方法：参数1：D:\jiaojie\mok\0.00.017\build\Mohacker\Bin\x64\Release为exe所在的目录，把此目录下的文件夹、dll、exe、png、ini、sensor_width_camera_database.txt、version.json全部复制一份，
::参数2：输出文件夹，输出文件夹不存在的话就创建该文件夹，并在该文件夹下创建: 版本号/newpackage/bin/目录，并将上述拷贝至该文件夹；版本号手动输入并同步更新到ini和version.json里