clear all;              %%��������ռ�����б���
for i=11:11
    file = ['D:\professional_install\matlab\bin\project_files\square\nanonodeposition', num2str(i), '.log'];
    fid = fopen(file);
    C = textscan(fid, '%f%f%f%f%f%f%f');          %%����fidΪfopen����ص��ļ���ʶ����formatʵ���Ͼ���һ���ַ�����������ʾ��ȡ���ݼ�����ת���Ĺ���
    fclose(fid);
    x = [C{3}];
    y = [C{4}];
    z = [C{5}];
    plot3(x, y, z);
    grid on;
    axis([0, 0.5, 0, 0.02, 0, 0.02]);       %�����������������ʾ��Χ
end