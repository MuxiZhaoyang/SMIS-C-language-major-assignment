/**
 * @file teachermanagement.cpp
 * @brief 此文件包含了 `teacherManagement` 类的定义，用于处理教师管理相关的功能
 */

#include "managerTerminal/headfile/teachermanagement.h"
#include "ui_teachermanagement.h"

/**
 * @brief `teacherManagement` 类的构造函数
 *
 * 初始化教师管理界面，并进行相关的数据模型、选择模型、映射等设置
 *
 * @param parent 父窗口指针
 * @param GDB 数据库指针
 */
teacherManagement::teacherManagement(QWidget *parent,QSqlDatabase * GDB)
    : QWidget(parent)
    , __GDB(GDB)
    , ui(new Ui::teacherManagement)
{
    ui->setupUi(this);

    //创建数据模型，打开数据表
    tabModel=new QSqlRelationalTableModel(this,*__GDB);	//数据模型
    tabModel->setTable("teacher");			//设置数据表
    tabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);	//数据保存方式，OnManualSubmit , OnRowChange
    tabModel->setSort(tabModel->fieldIndex("teacherID"),Qt::AscendingOrder); //排序
    if (!(tabModel->select()))	//查询数据失败
    {
        QMessageBox::critical(this, "错误信息",
                              "打开数据表错误,错误信息:\n"+tabModel->lastError().text());
        return;
    }

    //设置字段显示标题
    tabModel->setHeaderData(tabModel->fieldIndex("teacherID"),  Qt::Horizontal, "教师号");
    tabModel->setHeaderData(tabModel->fieldIndex("name"),  Qt::Horizontal, "姓名");
    tabModel->setHeaderData(tabModel->fieldIndex("schoolID"), Qt::Horizontal, "学院");

    //设置关联字段
    tabModel->setRelation(tabModel->fieldIndex("schoolID"),
                          QSqlRelation("school","schoolID","name"));            //学院

    //创建选择模型
    selModel=new QItemSelectionModel(tabModel,this);		//关联选择模型

    //当前行或列变化时发射currentChanged信号
    connect(selModel,&QItemSelectionModel::currentChanged,
            this,&teacherManagement::do_currentChanged);
    //当前行变化时发射currentRowChanged()信号
    connect(selModel,&QItemSelectionModel::currentRowChanged,
            this,&teacherManagement::do_currentRowChanged);

    //Model/View结构
    ui->tableView->setModel(tabModel);	//设置数据模型
    ui->tableView->setSelectionModel(selModel); //设置选择模型

    ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));  //为关系型字段设置默认代理组件

    //创建界面组件与数据模型的字段之间的数据映射
    dataMapper= new QDataWidgetMapper(this);
    dataMapper->setModel(tabModel);		//设置数据模型
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit); //数据更新方式

    //界面组件与数据模型的具体字段之间的联系
    dataMapper->addMapping(ui->dbSpinEmpNo,tabModel->fieldIndex("teacherID"));
    dataMapper->addMapping(ui->dbEditName,tabModel->fieldIndex("name"));
    dataMapper->toFirst();  //移动到首记录

    //获取字段名称列表，填充groupBoxSort
    getFieldNames();

    //界面组件的使能状态
    ui->actRecAppend->setEnabled(true);
    ui->actRecInsert->setEnabled(true);
    ui->actRecDelete->setEnabled(true);

    ui->groupBoxSort->setEnabled(true);		//"排序"分租框


    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);	//项选择
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);	//单项选择
    ui->tableView->setAlternatingRowColors(true);	//交错行底色

}

/**
 * @brief `teacherManagement` 类的析构函数
 */
teacherManagement::~teacherManagement()
{
    delete ui;
}

/**
 * @brief 获取所有字段名称
 */
void teacherManagement::getFieldNames()
{
    QSqlRecord  emptyRec=tabModel->record();    //获取空记录，只有字段名
    for (int i=0;i<emptyRec.count();i++)
        ui->comboFields->addItem(emptyRec.fieldName(i));
}

/**
 * @brief 数据发生修改，更新 actPost 和 actCancel 的状态
 *
 * @param current 当前的模型索引
 * @param previous 之前的模型索引
 */
void teacherManagement::do_currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
    ui->actSubmit->setEnabled(tabModel->isDirty());		//有未保存修改时可用
    ui->actRevert->setEnabled(tabModel->isDirty());
}

/**
 * @brief 当当前行改变时的处理函数
 *
 * @param current 当前的模型索引
 * @param previous 之前的模型索引
 */
void teacherManagement::do_currentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    // 行切换时的状态控制
    ui->actRecDelete->setEnabled(current.isValid());    //"删除"按钮

    dataMapper->setCurrentIndex(current.row());     //更细数据映射的行号

    int curRecNo=current.row();     //获取行号
    QSqlRecord  curRec=tabModel->record(curRecNo); //获取当前记录
}

/**
 * @brief 点击"添加记录"按钮的响应函数
 */
void teacherManagement::on_actRecAppend_clicked()
{//添加一条记录
    //// 使用QSqlRecord
    QSqlRecord rec=tabModel->record();      //获取一个空记录，只有字段定义
    rec.setValue(tabModel->fieldIndex("empNo"),2000+tabModel->rowCount());
    rec.setValue(tabModel->fieldIndex("Gender"),"男");  //设置数据
    tabModel->insertRecord(tabModel->rowCount(),rec);   //插入到数据模型的最后

    selModel->clearSelection();
    QModelIndex curIndex=tabModel->index(tabModel->rowCount()-1,1);     //创建最后一行的ModelIndex
    selModel->setCurrentIndex(curIndex,QItemSelectionModel::Select);
}

/**
 * @brief 点击"插入记录"按钮的响应函数
 */
void teacherManagement::on_actRecInsert_clicked()
{//插入一条记录
    //// 使用QSqlRecord
    QModelIndex curIndex=ui->tableView->currentIndex();
    QSqlRecord rec=tabModel->record();      //获取一个空记录，只有字段定义
    tabModel->insertRecord(curIndex.row(),rec);     //在当前行前面插入一条记录

    selModel->clearSelection();
    selModel->setCurrentIndex(curIndex,QItemSelectionModel::Select);    //设置当前行
}

/**
 * @brief 点击"保存修改"按钮的响应函数
 */
void teacherManagement::on_actSubmit_clicked()
{//保存修改
    bool res=tabModel->submitAll();
    if (!res)
        QMessageBox::information(this, "消息", "数据保存错误,错误信息\n"+tabModel->lastError().text());
    else
    {
        ui->actSubmit->setEnabled(false);
        ui->actRevert->setEnabled(false);
    }
}

/**
 * @brief 点击"取消修改"按钮的响应函数
 */
void teacherManagement::on_actRevert_clicked()
{//取消修改
    tabModel->revertAll();
    ui->actSubmit->setEnabled(false);
    ui->actRevert->setEnabled(false);
}

/**
 * @brief 点击"删除当前记录"按钮的响应函数
 */
void teacherManagement::on_actRecDelete_clicked()
{//删除当前记录
    QModelIndex curIndex=selModel->currentIndex();  //获取当前选择单元格的模型索引
    tabModel->removeRow(curIndex.row()); //删除当前行
}

/**
 * @brief 点击"升序排序"按钮的响应函数
 */
void teacherManagement::on_radioBtnAscend_clicked()
{//升序排序
    tabModel->setSort(ui->comboFields->currentIndex(),Qt::AscendingOrder);
    tabModel->select();     //setSort()之后需要执行select()才会刷新数据
}

/**
 * @brief 点击"降序排序"按钮的响应函数
 */
void teacherManagement::on_radioBtnDescend_clicked()
{//降序排序
    tabModel->sort(ui->comboFields->currentIndex(),Qt::DescendingOrder);
}

/**
 * @brief 选择字段进行排序时的响应函数
 *
 * @param index 选择的字段索引
 */
void teacherManagement::on_comboFields_currentIndexChanged(int index)
{//选择字段进行排序
    if (ui->radioBtnAscend->isChecked())
        tabModel->setSort(index,Qt::AscendingOrder);
    else
        tabModel->setSort(index,Qt::DescendingOrder);

    tabModel->select();
}

/**
 * @brief 当搜索框文本改变时的响应函数
 *
 * @param arg1 搜索框的当前文本
 */
void teacherManagement::on_lineEdit_textChanged(const QString &arg1)
{//搜索姓名
    tabModel->setFilter(QString(" teacherID LIKE '%1%' ").arg(arg1));
}
