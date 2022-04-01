clear all;              %%��������ռ�����б���
linearWidth = 1.2;      %�������
all_nodeNums = [10; 20; 30; 40; 50; 60; 70; 80; 90; 100];
filesNum = 30;          %ʵ��Ĵ���
nodeNum = 10;           %ʵ��ڵ�Ĵ���
zValue = 1.96;          %���������zֵ
duration = 15;          %������ʱ�� 
%%����20��ʵ�飬ÿ��ʵ��ȡ10����
result1 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
Variance1 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
for i = 1:filesNum
    for j=1:nodeNum
        file1 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_phyTx',num2str(j), '.log'];
        fid1 = fopen(file1);
        C1 = textscan(fid1, '%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid1);
        data1 = [C1{1}];                         %%ȡ����1��Ԫ��
        len1 = length(data1);
        file2 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_Rx', num2str(j), '.log'];
        fid2 = fopen(file2);
        C2 = textscan(fid2, '%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid2);
        data2 = [C2{7}];                        %%ȡ����7��Ԫ��
        len2 = length(data2); 
        result1(j) = (len1 / len2) + result1(j);   
    end
end
result1 = result1./filesNum;            %�����ֵ���飬Ȼ���������׼���
for i = 1:filesNum
    for j=1:nodeNum
        file1 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_phyTx',num2str(j), '.log'];
        fid1 = fopen(file1);
        C1 = textscan(fid1, '%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid1);
        data1 = [C1{1}];                         %%ȡ����1��Ԫ��
        len1 = length(data1);
        file2 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\flooding_Rx', num2str(j), '.log'];
        fid2 = fopen(file2);
        C2 = textscan(fid2, '%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid2);
        data2 = [C2{7}];                        %%ȡ����7��Ԫ��
        len2 = length(data2);
        Variance1(j) = ((len1 / len2) - result1(j))^2 + Variance1(j);
    end
end
Variance1 = sqrt(Variance1)./filesNum*zValue;
errorbar(all_nodeNums(:,1), result1(:,1),Variance1, '-->g','LineWidth',linearWidth);
% ylabel('Average hops(times)');
% xlabel('Number of nano-nodes(n)');
ylabel('���ݰ�����ƽ������(��)');
xlabel('���׽ڵ�����(��)');
hold on


result2 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
Variance2 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
for i = 1:filesNum
    for j=1:nodeNum
        file3 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_phyTx',num2str(j), '.log'];
        fid3 = fopen(file3);
        C3 = textscan(fid3, '%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid3);
        data3 = [C3{1}];                         %%ȡ����1��Ԫ��
        len3 = length(data3);
        file4 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_Rx', num2str(j), '.log'];
        fid4 = fopen(file4);
        C4 = textscan(fid4, '%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid4);
        data4 = [C4{7}];                        %%ȡ����7��Ԫ��
        len4 = length(data4); 
        result2(j) = (len3 / len4) + result2(j); 
    end
end
result2 = result2./filesNum;            %�����ֵ���飬Ȼ���������׼���
for i = 1:filesNum
    for j=1:nodeNum
        file3 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_phyTx',num2str(j), '.log'];
        fid3 = fopen(file3);
        C3 = textscan(fid3, '%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid3);
        data3 = [C3{1}];                         %%ȡ����1��Ԫ��
        len3 = length(data3);
        file4 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\random_Rx', num2str(j), '.log'];
        fid4 = fopen(file4);
        C4 = textscan(fid4, '%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid4);
        data4 = [C4{7}];                        %%ȡ����7��Ԫ��
        len4 = length(data4); 
        Variance2(j) = ((len3 / len4) - result2(j))^2 + Variance2(j);
    end
end
Variance2 = sqrt(Variance2)./filesNum*zValue;
errorbar(all_nodeNums(:,1), result2(:,1),Variance2, '--db','LineWidth',linearWidth);
hold on


result3 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
Variance3 = [0; 0; 0; 0; 0; 0; 0; 0; 0; 0];
for i = 1:filesNum
    for j=1:nodeNum
        file5 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_phyTx',num2str(j), '.log'];
        fid5 = fopen(file5);
        C5 = textscan(fid5, '%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid5);
        temp = [C5{1}];                         %%ȡ����1��Ԫ��
        idx = find(temp(:,1)~=0);               %%ȡ������Ԫ��
        data5 = temp(idx,:);
        len5 = length(data5);
        file6 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_Rx', num2str(j), '.log'];
        fid6 = fopen(file6);
        C6 = textscan(fid6, '%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid6);
        data6 = [C6{7}];                        %%ȡ����7��Ԫ��
        len6 = length(data6); 
        result3(j) = (len5 / len6) + result3(j); 
    end
end
result3 = result3./filesNum;            %�����ֵ���飬Ȼ���������׼���
for i = 1:filesNum
    for j=1:nodeNum
        file5 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_phyTx',num2str(j), '.log'];
        fid5 = fopen(file5);
        C5 = textscan(fid5, '%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid5);
        temp = [C5{1}];                         %%ȡ����1��Ԫ��
        idx = find(temp(:,1)~=0);               %%ȡ������Ԫ��
        data5 = temp(idx,:);
        len5 = length(data5);
        file6 = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_Rx', num2str(j), '.log'];
        fid6 = fopen(file6);
        C6 = textscan(fid6, '%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid6);
        data6 = [C6{7}];                        %%ȡ����7��Ԫ��
        len6 = length(data6); 
        Variance3(j) = ((len5 / len6) - result3(j))^2 + Variance3(j);
    end
end
Variance3 = sqrt(Variance3)./filesNum*zValue;
errorbar(all_nodeNums(:,1), result3(:,1),Variance3, '--pr','LineWidth',linearWidth);
legend('Flooding', 'Random', 'COR','Location','northwest');




