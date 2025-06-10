// 测试清理数据功能
const ticketController = require('./controllers/ticketController');

async function testClean() {
  console.log('正在测试清理数据功能...');
  
  try {
    // 调用clean命令
    const result = await ticketController.clean();
    console.log('清理结果:', result);
    
    if (result.success) {
      console.log('✅ 数据清理成功！');
    } else {
      console.log('❌ 数据清理失败:', result.response);
    }
  } catch (error) {
    console.error('❌ 清理过程中发生错误:', error.message);
  }
  
  // 退出进程
  process.exit(0);
}

testClean();
