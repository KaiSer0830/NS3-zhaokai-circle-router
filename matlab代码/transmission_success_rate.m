clear all;              %%清除工作空间的所有变量
linearWidth = 1.2;      %线条宽度
all_nodeNums = [10; 20; 30; 40; 50; 60; 70; 80; 90; 100];
filesNum = 30;           %实验的次数
nodeNum = 10;           %实验节点的次数
zValue = 1.96;          %置信区间的z值
%%做了20组实验，每组实验取10个点
result1 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
Variance1 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
for i = 1:filesNum
    for j=1:nodeNum
        file1 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_Rx',num2str(j), '.log'];
        fid1 = fopen(file1);
        C1 = textscan(fid1, '%f%f%f%f%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid1);
        dataRx1 = [C1{7}];                         %%取出第7个元素
        file2 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_Tx', num2str(j), '.log'];
        fid2 = fopen(file2);
        C2 = textscan(fid2, '%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid2);
        dataTx1 = [C2{3}];                         %%取出第3个元素
        result1(j) = (length(dataRx1) / length(dataTx1)) * 100 + result1(j);  
    end
end
result1 = result1./filesNum;            %求出均值数组，然后下面求标准误差
for i = 1:filesNum
    for j=1:nodeNum
        file1 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_Rx',num2str(j), '.log'];
        fid1 = fopen(file1);
        C1 = textscan(fid1, '%f%f%f%f%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid1);
        dataRx1 = [C1{7}];                         %%取出第7个元素
        file2 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_Tx', num2str(j), '.log'];
        fid2 = fopen(file2);
        C2 = textscan(fid2, '%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid2);
        dataTx1 = [C2{3}];                         %%取出第3个元素
        Variance1(j) = ((length(dataRx1) / length(dataTx1) * 100) - result1(j))^2 + Variance1(j);
    end
end
Variance1 = sqrt(Variance1)./filesNum*zValue;
errorbar(all_nodeNums(:,1), result1(:,1),Variance1, '-->g','LineWidth',linearWidth);
% ylabel('Transmission success rate(%)');
% xlabel('Number of nano-nodes(n)');
ylabel('数据包成功传输率(%)');
xlabel('纳米节点数量(个)');
hold on


result2 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
Variance2 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
for i = 1:filesNum
    for j=1:nodeNum
        file3 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_Rx',num2str(j), '.log'];
        fid3 = fopen(file3);
        C3 = textscan(fid3, '%f%f%f%f%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid3);
        dataRx2 = [C3{7}];                         %%取出第7个元素
        file4 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_Tx', num2str(j), '.log'];
        fid4 = fopen(file4);
        C4 = textscan(fid4, '%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid4);
        dataTx2 = [C4{3}];                         %%取出第3个元素
        result2(j) = (length(dataRx2) / length(dataTx2) * 100) + result2(j);  
    end
end
result2 = result2./filesNum;            %求出均值数组，然后下面求标准误差
for i = 1:filesNum
    for j=1:nodeNum
        file3 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_Rx',num2str(j), '.log'];
        fid3 = fopen(file3);
        C3 = textscan(fid3, '%f%f%f%f%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid3);
        dataRx2 = [C3{7}];                         %%取出第7个元素
        file4 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_Tx', num2str(j), '.log'];
        fid4 = fopen(file4);
        C4 = textscan(fid4, '%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid4);
        dataTx2 = [C4{3}];                         %%取出第3个元素
        Variance2(j) = ((length(dataRx2) / length(dataTx2) * 100) - result2(j))^2 + Variance2(j);
    end
end
Variance2 = sqrt(Variance2)./filesNum*zValue;
errorbar(all_nodeNums(:,1), result2(:,1),Variance2, '--db','LineWidth',linearWidth);
hold on


result3 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
Variance3 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
for i = 1:filesNum
    for j=1:nodeNum
        file5 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_Rx',num2str(j), '.log'];
        fid5 = fopen(file5);
        C5 = textscan(fid5, '%f%f%f%f%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid5);
        dataRx3 = [C5{7}];                         %%取出第7个元素
        file6 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_Tx', num2str(j), '.log'];
        fid6 = fopen(file6);
        C6 = textscan(fid6, '%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid6);
        dataTx3 = [C6{3}];                         %%取出第3个元素
        result3(j) = (length(dataRx3) / length(dataTx3) * 100) + result3(j); 
    end
end
result3 = result3./filesNum;            %求出均值数组，然后下面求标准误差
for i = 1:filesNum
    for j=1:nodeNum
        file5 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_Rx',num2str(j), '.log'];
        fid5 = fopen(file5);
        C5 = textscan(fid5, '%f%f%f%f%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid5);
        dataRx3 = [C5{7}];                         %%取出第7个元素
        file6 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_Tx', num2str(j), '.log'];
        fid6 = fopen(file6);
        C6 = textscan(fid6, '%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid6);
        dataTx3 = [C6{3}];                         %%取出第3个元素
        Variance3(j) = ((length(dataRx3) / length(dataTx3) * 100) - result3(j))^2 + Variance3(j);
    end
end
Variance3 = sqrt(Variance3)./filesNum*zValue;
errorbar(all_nodeNums(:,1), result3(:,1),Variance3, '--pr','LineWidth',linearWidth);
legend('Flooding', 'Random', 'COR','Location','northeast');




