clear all;              %%清除工作空间的所有变量
all_data = [];
filesNum = 10;          %实验的次数
nodeNum = 10;           %实验节点的次数
for i=1:filesNum
    for j=1:nodeNum
        file = ['D:\professional_install\matlab\bin\project_files\opport_circle_zhao\log', num2str(i),'\opport_nanonodeposition',num2str(j), '.log'];
        fid = fopen(file);
        C = textscan(fid, '%f%f%f%f%f%f%s%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
        fclose(fid);
        data = [C{1}, C{8}];                         %%取出第1个与第8个元素
        data = data(100:100:end, :);                %%逗号前面代表行，后面代表列。第一个元素代表取的起始号，为第200行；第二个元素代表取的间隔数200，第二个数取第400行；第三个元素代表取到末尾。
        all_data = [all_data; data];      
    end
end

all_data = all_data(:,2:2:end);             %%只取列
idx = find(all_data(:,1)~=0);               %%取非零列元素
data2 = all_data(idx,:);

x = 1e-10:1e-9:1e-6;
y = zeros(1,length(x));
for i=1:length(y)
    y(i) = size(find(data2 < x(i)), 1);
end
y = y./29002;
plot(x, y, '-r', 'LineWidth', 2);
% xlabel('Temperatrue increase ΔT(℃)');
% ylabel('Cumulative distribution function(CDF)');
xlabel('上升温度ΔT(℃)');
ylabel('积累分布函数(CDF)');
legend('simulation of CDF')
%hold on作用是保持原图并接受此后绘制的新的曲线,叠加绘图; 
hold on


