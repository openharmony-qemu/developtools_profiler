# 关于patch文件的生成和使用
## 需要patch的场景
所谓patch文件，是一个文件相比另外一个文件的变化内容。
项目在如下情况下时，可能会遇到需要给文件打patch的情况。  
当开发者从某个指定地址下载的文件，不能直接使用，需要略做修改，而如果直接把修改后的文件全量上传到库，涉及代码和第三方的相似度高的问题。  
而生成patch文件，则可以避免这个问题。  
生成patch文件  
diff -Naru old_file.xx new_file.xx > patch.xx  
给文件打patch  
patch old_file.xx patch.xx
这个命令，可以把old_file.xx变成和new_file.xx一样的内容。  
## 需要sed的场景
还有些情况下，开发者非常确认我们要修改的内容的特性，比如指定的方法，指定的标签特性。此时，可以用sed命令做查找和替换。