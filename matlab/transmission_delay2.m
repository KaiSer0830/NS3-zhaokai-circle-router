clear all;              %%��������ռ�����б���
all_nodeNums = [10; 20; 30; 40; 50; 60; 70; 80; 90; 100];
filesNum = 1;          %ʵ��Ĵ���
nodeNum = 10;           %ʵ��ڵ�Ĵ���
zValue = 1.96;          %���������zֵ
duration = 15;          %������ʱ�� 
%%����10��ʵ�飬ÿ��ʵ��ȡ10����
result1 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
Variance1 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
for i = 1:filesNum
    for j=1:nodeNum
        file1 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_Rx', num2str(j), '.log'];
        fid1 = fopen(file1);
        C1 = textscan(fid1, '%f%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid1);
        data1 = [C1{5}];                         %%ȡ����5��Ԫ��
        s1 = sum(data1);
        dataD1 = [C1{6}];
        sD1 = sum(dataD1);
        len1 = length(data1); 
        result1(j) = (sD1 / s1) + result1(j); 
    end
end
result1 = result1./filesNum;            %�����ֵ���飬Ȼ���������׼���
for i = 1:filesNum
    for j=1:nodeNum
        file1 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_Rx', num2str(j), '.log'];
        fid1 = fopen(file1);
        C1 = textscan(fid1, '%f%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid1);
        data1 = [C1{5}];                         %%ȡ����5��Ԫ��
        s1 = sum(data1);
        dataD1 = [C1{6}];
        sD1 = sum(dataD1);
        len1 = length(data1);  
        Variance1(j) = ((sD1 / s1) - result1(j))^2 + Variance1(j);
    end
end
Variance1 = sqrt(Variance1)./filesNum*zValue;
errorbar(all_nodeNums(:,1), result1(:,1),Variance1, '-->g');
ylabel('Delay(s)');
xlabel('Density of Nodes');
hold on


result2 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
Variance2 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
for i = 1:filesNum
    for j=1:nodeNum
        file2 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_Rx', num2str(j), '.log'];
        fid2 = fopen(file2);
        C2 = textscan(fid2, '%f%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid2);
        data2 = [C2{5}];                         %%ȡ����5��Ԫ��
        s2 = sum(data2);
        dataD2 = [C2{6}];
        sD2 = sum(dataD2);
        len2 = length(data2);
        result2(j) = (sD2 / s2) + result2(j); 
    end
end
result2 = result2./filesNum;            %�����ֵ���飬Ȼ���������׼���
for i = 1:filesNum
    for j=1:nodeNum
        file2 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_Rx', num2str(j), '.log'];
        fid2 = fopen(file2);
        C2 = textscan(fid2, '%f%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid2);
        data2 = [C2{5}];                         %%ȡ����5��Ԫ��
        s2 = sum(data2);
        dataD2 = [C2{6}];
        sD2 = sum(dataD2);
        len2 = length(data2);
        Variance2(j) = ((sD2 / s2) - result2(j))^2 + Variance2(j);
    end
end
Variance2 = sqrt(Variance2)./filesNum*zValue;
errorbar(all_nodeNums(:,1), result2(:,1),Variance2, '--db');
hold on


result3 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
Variance3 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
for i = 1:filesNum
    for j=1:nodeNum
        file3 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_Rx', num2str(j), '.log'];
        fid3 = fopen(file3);
        C3 = textscan(fid3, '%f%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid3);
        data3 = [C3{5}];                         %%ȡ����5��Ԫ��
        s3 = sum(data3);
        dataD3 = [C3{6}];
        sD3 = sum(dataD3);
        len3 = length(data3);
        result3(j) = (sD3 / s3) + result3(j);
    end
end
result3 = result3./filesNum;            %�����ֵ���飬Ȼ���������׼���
for i = 1:filesNum
    for j=1:nodeNum
        file3 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_Rx', num2str(j), '.log'];
        fid3 = fopen(file3);
        C3 = textscan(fid3, '%f%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid3);
        data3 = [C3{5}];                         %%ȡ����5��Ԫ��
        s3 = sum(data3);
        dataD3 = [C3{6}];
        sD3 = sum(dataD3);
        len3 = length(data3);
        Variance3(j) = ((sD3 / s3) - result3(j))^2 + Variance3(j);
    end
end
Variance3 = sqrt(Variance3)./filesNum*zValue;
errorbar(all_nodeNums(:,1), result3(:,1),Variance3, '--pr');
legend('Flooding', 'Random', 'Opport');




