#!/usr/bin/env node

// 测试后端协议是否符合 acquirement.md 规范
const ticketController = require('./controllers/ticketController');

async function testBackendProtocol() {
  console.log('=== 测试后端协议符合性 ===\n');
  
  try {
    // 测试 1: 用户注册 (add_user) 
    console.log('1. 测试用户注册...');
    const addUserResult = await ticketController.addUser(null, 'testuser', 'password123', '测试用户', 'test@example.com', 1);
    console.log('   注册结果:', addUserResult);
    
    // 测试 2: 用户登录 (login)
    console.log('\n2. 测试用户登录...');
    const loginResult = await ticketController.login('testuser', 'password123');
    console.log('   登录结果:', loginResult);
    
    // 测试 3: 查询用户信息 (query_profile)
    console.log('\n3. 测试查询用户信息...');
    const profileResult = await ticketController.queryProfile('testuser', 'testuser');
    console.log('   查询结果:', profileResult);
    
    // 测试 4: 查询车票 (query_ticket)
    console.log('\n4. 测试查询车票...');
    const ticketResult = await ticketController.queryTicket('Beijing', 'Shanghai', '07-01', 'time');
    console.log('   查询结果:', ticketResult);
    console.log('   车票数量:', ticketResult.tickets?.length || 0);
    
    // 测试 5: 购买车票 (buy_ticket)
    console.log('\n5. 测试购买车票...');
    const buyResult = await ticketController.buyTicket('testuser', 'G123', '07-01', 1, 'Beijing', 'Shanghai');
    console.log('   购买结果:', buyResult);
    
    // 测试 6: 查询订单 (query_order)
    console.log('\n6. 测试查询订单...');
    const orderResult = await ticketController.queryOrder('testuser');
    console.log('   订单结果:', orderResult);
    console.log('   订单数量:', orderResult.orders?.length || 0);
    
    // 测试 7: 用户登出 (logout)
    console.log('\n7. 测试用户登出...');
    const logoutResult = await ticketController.logout('testuser');
    console.log('   登出结果:', logoutResult);
    
    console.log('\n=== 协议测试完成 ===');
    console.log('✅ 所有测试均已执行，请检查上述结果是否符合 acquirement.md 规范');
    
  } catch (error) {
    console.error('❌ 测试过程中出现错误:', error);
  }
  
  // 退出进程
  process.exit(0);
}

// 延迟启动，等待控制器初始化
setTimeout(testBackendProtocol, 2000);
