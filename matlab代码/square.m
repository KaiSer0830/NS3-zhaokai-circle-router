clear all;              %%清除工作空间的所有变量
for i=11:11
    file = ['D:\professional_install\matlab\bin\project_files\square\nanonodeposition', num2str(i), '.log'];
    fid = fopen(file);
    C = textscan(fid, '%f%f%f%f%f%f%f');          %%其中fid为fopen命令返回的文件标识符，format实际上就是一个字符串变量，表示读取数据及数据转换的规则。
    fclose(fid);
    x = [C{3}];
    y = [C{4}];
    z = [C{5}];
    plot3(x, y, z);
    grid on;
    axis([0, 0.5, 0, 0.02, 0, 0.02]);       %设置三个坐标轴的显示范围
end