clear all;              %%��������ռ�����б���
all_data = [];
filesNum = 10;          %ʵ��Ĵ���
nodeNum = 10;           %ʵ��ڵ�Ĵ���
for i=1:filesNum
    for j=1:nodeNum
        file = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_nanonodeposition',num2str(j), '.log'];
        fid = fopen(file);
        C = textscan(fid, '%f%f%f%f%f%f%s%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
        fclose(fid);
        data = [C{1}, C{8}];                         %%ȡ����1�����8��Ԫ��
        data = data(100:100:end, :);                %%����ǰ������У���������С���һ��Ԫ�ش���ȡ����ʼ�ţ�Ϊ��200�У��ڶ���Ԫ�ش���ȡ�ļ����200���ڶ�����ȡ��400�У�������Ԫ�ش���ȡ��ĩβ��
        all_data = [all_data; data];      
    end
end

all_data = all_data(:,2:2:end);             %%ֻȡ��
idx = find(all_data(:,1)~=0);               %%ȡ������Ԫ��
data2 = all_data(idx,:);

x = 1e-10:1e-9:1e-6;
y = zeros(1,length(x));
for i=1:length(y)
    y(i) = size(find(data2 < x(i)), 1);
end
y = y./29002;
plot(x, y, '-r', 'LineWidth', 2);
% xlabel('Temperatrue increase ��T(��)');
% ylabel('Cumulative distribution function(CDF)');
xlabel('�����¶Ȧ�T(��)');
ylabel('���۷ֲ�����(CDF)');
legend('simulation of CDF')
%hold on�����Ǳ���ԭͼ�����ܴ˺���Ƶ��µ�����,���ӻ�ͼ; 
hold on


