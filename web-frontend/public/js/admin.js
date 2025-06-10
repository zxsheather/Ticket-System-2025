/**
 * 管理员面板JavaScript
 * 处理管理员界面的交互和实时更新
 */

class AdminPanel {
  constructor() {
    this.socket = null;
    this.currentTab = 'dashboard';
    this.systemStatus = null;
    this.refreshInterval = null;
    this.charts = {};
    
    this.init();
  }

  /**
   * 初始化管理面板
   */
  init() {
    this.initializeSocket();
    this.setupEventListeners();
    this.setupCharts();
    this.startAutoRefresh();
    
    console.log('管理员面板已初始化');
  }

  /**
   * 初始化WebSocket连接
   */
  initializeSocket() {
    this.socket = io();
    
    // 连接成功
    this.socket.on('connect', () => {
      console.log('WebSocket连接成功');
      
      // 认证管理员身份
      const userData = {
        username: window.currentUser?.username,
        privilege: window.currentUser?.privilege || 0
      };
      
      this.socket.emit('user_auth', userData);
      this.socket.emit('join_room', 'admin');
    });

    // 认证成功
    this.socket.on('auth_success', (data) => {
      console.log('管理员认证成功:', data);
      this.updateConnectionStatus(true);
    });

    // 系统状态更新
    this.socket.on('system_status', (status) => {
      this.updateSystemStatus(status);
    });

    // 管理员通知
    this.socket.on('admin_notification', (notification) => {
      this.showNotification(notification);
    });

    // 管理员广播
    this.socket.on('admin_broadcast', (data) => {
      this.showNotification({
        title: '系统广播',
        message: data.message,
        level: data.level || 'info'
      });
    });

    // 连接错误
    this.socket.on('connect_error', (error) => {
      console.error('WebSocket连接错误:', error);
      this.updateConnectionStatus(false);
    });

    // 断开连接
    this.socket.on('disconnect', () => {
      console.log('WebSocket连接断开');
      this.updateConnectionStatus(false);
    });
  }

  /**
   * 设置事件监听器
   */
  setupEventListeners() {
    // 表单提交处理
    this.setupFormHandlers();
    
    // 模态框处理
    this.setupModalHandlers();
    
    // 实时搜索
    this.setupSearchHandlers();
    
    // 批量操作
    this.setupBatchActions();
    
    // 添加列车表单处理器
    this.setupTrainFormHandlers();
  }

  /**
   * 设置表单处理器
   */
  setupFormHandlers() {
    // 添加列车表单
    const addTrainForm = document.getElementById('addTrainForm');
    if (addTrainForm) {
      addTrainForm.addEventListener('submit', (e) => {
        e.preventDefault();
        this.handleAddTrain(e.target);
      });
    }

    // 广播消息表单
    const broadcastForm = document.getElementById('broadcastForm');
    if (broadcastForm) {
      broadcastForm.addEventListener('submit', (e) => {
        e.preventDefault();
        this.handleBroadcastMessage(e.target);
      });
    }

    // 用户管理表单
    const userManageForm = document.getElementById('userManageForm');
    if (userManageForm) {
      userManageForm.addEventListener('submit', (e) => {
        e.preventDefault();
        this.handleUserAction(e.target);
      });
    }
  }

  /**
   * 设置模态框处理器
   */
  setupModalHandlers() {
    // 删除确认模态框
    const deleteModal = document.getElementById('deleteModal');
    if (deleteModal) {
      const confirmBtn = deleteModal.querySelector('.btn-danger');
      if (confirmBtn) {
        confirmBtn.addEventListener('click', () => {
          this.confirmDelete();
        });
      }
    }

    // 用户详情模态框
    const userDetailModal = document.getElementById('userDetailModal');
    if (userDetailModal) {
      userDetailModal.addEventListener('show.bs.modal', (e) => {
        const username = e.relatedTarget?.getAttribute('data-username');
        if (username) {
          this.loadUserDetails(username);
        }
      });
    }
  }

  /**
   * 设置搜索处理器
   */
  setupSearchHandlers() {
    // 用户搜索
    const userSearchInput = document.getElementById('userSearch');
    if (userSearchInput) {
      let searchTimeout;
      userSearchInput.addEventListener('input', (e) => {
        clearTimeout(searchTimeout);
        searchTimeout = setTimeout(() => {
          this.searchUsers(e.target.value);
        }, 300);
      });
    }

    // 订单搜索
    const orderSearchInput = document.getElementById('orderSearch');
    if (orderSearchInput) {
      let searchTimeout;
      orderSearchInput.addEventListener('input', (e) => {
        clearTimeout(searchTimeout);
        searchTimeout = setTimeout(() => {
          this.searchOrders(e.target.value);
        }, 300);
      });
    }

    // 列车搜索
    const trainSearchInput = document.getElementById('trainSearch');
    if (trainSearchInput) {
      let searchTimeout;
      trainSearchInput.addEventListener('input', (e) => {
        clearTimeout(searchTimeout);
        searchTimeout = setTimeout(() => {
          this.searchTrains(e.target.value);
        }, 300);
      });
    }
  }

  /**
   * 设置批量操作
   */
  setupBatchActions() {
    // 全选复选框
    const selectAllCheckbox = document.getElementById('selectAll');
    if (selectAllCheckbox) {
      selectAllCheckbox.addEventListener('change', (e) => {
        const checkboxes = document.querySelectorAll('.item-checkbox');
        checkboxes.forEach(cb => cb.checked = e.target.checked);
        this.updateBatchActions();
      });
    }

    // 批量操作按钮
    const batchDeleteBtn = document.getElementById('batchDelete');
    if (batchDeleteBtn) {
      batchDeleteBtn.addEventListener('click', () => {
        this.batchDelete();
      });
    }
  }

  /**
   * 设置添加列车表单处理器
   */
  setupTrainFormHandlers() {
    const stationNumInput = document.getElementById('stationNum');
    if (stationNumInput) {
      stationNumInput.addEventListener('change', (e) => {
        const stationCount = parseInt(e.target.value);
        this.generateStationInputs(stationCount);
      });
    }

    // 订单日期过滤器处理 - 确保日期格式正确
    const dateFilter = document.getElementById('dateFilter');
    if (dateFilter) {
      dateFilter.addEventListener('change', (e) => {
        const selectedDate = e.target.value; // YYYY-MM-DD format
        if (selectedDate) {
          // 转换为MM-DD格式
          const date = new Date(selectedDate);
          const month = String(date.getMonth() + 1).padStart(2, '0');
          const day = String(date.getDate()).padStart(2, '0');
          const mmddFormat = `${month}-${day}`;
          this.filterOrdersByDate(mmddFormat);
        } else {
          this.loadOrders(); // 清除过滤器时重新加载所有订单
        }
      });
    }
  }

  /**
   * 动态生成车站输入框
   */
  generateStationInputs(stationCount) {
    const container = document.getElementById('stationsContainer');
    if (!container) return;

    // 清空现有内容
    container.innerHTML = '';

    if (stationCount < 2) return;

    // 生成车站信息标题
    const stationsTitle = document.createElement('h6');
    stationsTitle.className = 'mb-3 text-primary';
    stationsTitle.innerHTML = '<i class="fas fa-map-marker-alt me-2"></i>车站信息';
    container.appendChild(stationsTitle);

    // 生成车站输入框
    for (let i = 0; i < stationCount; i++) {
      const row = document.createElement('div');
      row.className = 'row mb-3';
      
      const stationCol = document.createElement('div');
      stationCol.className = 'col-md-4';
      stationCol.innerHTML = `
        <label class="form-label">第${i + 1}站</label>
        <input type="text" class="form-control station-input" 
               name="station_${i}" placeholder="车站名称" required>
      `;
      
      const priceCol = document.createElement('div');
      priceCol.className = 'col-md-3';
      if (i < stationCount - 1) {
        priceCol.innerHTML = `
          <label class="form-label">到下一站票价</label>
          <input type="number" class="form-control price-input" 
                 name="price_${i}" placeholder="票价" min="0" step="0.01" required>
        `;
      }
      
      const travelTimeCol = document.createElement('div');
      travelTimeCol.className = 'col-md-3';
      if (i < stationCount - 1) {
        travelTimeCol.innerHTML = `
          <label class="form-label">到下一站时间(分钟)</label>
          <input type="number" class="form-control travel-time-input" 
                 name="travelTime_${i}" placeholder="行驶时间" min="0" required>
        `;
      }
      
      const stopoverCol = document.createElement('div');
      stopoverCol.className = 'col-md-2';
      if (i > 0 && i < stationCount - 1) {
        stopoverCol.innerHTML = `
          <label class="form-label">停靠时间(分钟)</label>
          <input type="number" class="form-control stopover-input" 
                 name="stopover_${i}" placeholder="停靠时间" min="0" value="5">
        `;
      }
      
      row.appendChild(stationCol);
      row.appendChild(priceCol);
      row.appendChild(travelTimeCol);
      row.appendChild(stopoverCol);
      
      container.appendChild(row);
    }
  }

  /**
   * 设置图表
   */
  setupCharts() {
    // 系统性能图表
    this.setupPerformanceChart();
    
    // 用户活动图表
    this.setupUserActivityChart();
    
    // 订单统计图表
    this.setupOrderChart();
  }

  /**
   * 设置性能图表
   */
  setupPerformanceChart() {
    const ctx = document.getElementById('performanceChart');
    if (!ctx) return;

    this.charts.performance = new Chart(ctx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [{
          label: 'CPU使用率 (%)',
          data: [],
          borderColor: 'rgb(255, 99, 132)',
          backgroundColor: 'rgba(255, 99, 132, 0.2)',
          tension: 0.1
        }, {
          label: '内存使用率 (%)',
          data: [],
          borderColor: 'rgb(54, 162, 235)',
          backgroundColor: 'rgba(54, 162, 235, 0.2)',
          tension: 0.1
        }]
      },
      options: {
        responsive: true,
        scales: {
          y: {
            beginAtZero: true,
            max: 100
          }
        },
        plugins: {
          title: {
            display: true,
            text: '系统性能监控'
          }
        }
      }
    });
  }

  /**
   * 设置用户活动图表
   */
  setupUserActivityChart() {
    const ctx = document.getElementById('userActivityChart');
    if (!ctx) return;

    this.charts.userActivity = new Chart(ctx, {
      type: 'bar',
      data: {
        labels: ['登录', '注册', '购票', '查询', '退票'],
        datasets: [{
          label: '今日活动',
          data: [0, 0, 0, 0, 0],
          backgroundColor: [
            'rgba(255, 99, 132, 0.8)',
            'rgba(54, 162, 235, 0.8)',
            'rgba(255, 205, 86, 0.8)',
            'rgba(75, 192, 192, 0.8)',
            'rgba(153, 102, 255, 0.8)'
          ]
        }]
      },
      options: {
        responsive: true,
        plugins: {
          title: {
            display: true,
            text: '用户活动统计'
          }
        }
      }
    });
  }

  /**
   * 设置订单图表
   */
  setupOrderChart() {
    const ctx = document.getElementById('orderChart');
    if (!ctx) return;

    this.charts.order = new Chart(ctx, {
      type: 'doughnut',
      data: {
        labels: ['已支付', '待支付', '已取消', '已退票'],
        datasets: [{
          data: [0, 0, 0, 0],
          backgroundColor: [
            'rgba(75, 192, 192, 0.8)',
            'rgba(255, 205, 86, 0.8)',
            'rgba(255, 99, 132, 0.8)',
            'rgba(153, 102, 255, 0.8)'
          ]
        }]
      },
      options: {
        responsive: true,
        plugins: {
          title: {
            display: true,
            text: '订单状态分布'
          }
        }
      }
    });
  }

  /**
   * 开始自动刷新
   */
  startAutoRefresh() {
    this.refreshInterval = setInterval(() => {
      if (this.currentTab === 'dashboard') {
        this.refreshDashboard();
      }
    }, 30000); // 30秒刷新一次
  }

  /**
   * 显示标签页
   */
  showTab(tabName) {
    // 隐藏所有标签页内容
    document.querySelectorAll('.tab-content').forEach(tab => {
      tab.style.display = 'none';
    });

    // 移除所有活动状态
    document.querySelectorAll('.list-group-item').forEach(item => {
      item.classList.remove('active');
    });

    // 显示目标标签页
    const targetTab = document.getElementById(tabName);
    if (targetTab) {
      targetTab.style.display = 'block';
    }

    // 设置活动状态
    const targetLink = document.querySelector(`[onclick="showTab('${tabName}')"]`);
    if (targetLink) {
      targetLink.classList.add('active');
    }

    this.currentTab = tabName;

    // 根据标签页执行特定操作
    switch (tabName) {
      case 'dashboard':
        this.refreshDashboard();
        break;
      case 'users':
        this.loadUsers();
        break;
      case 'orders':
        this.loadOrders();
        break;
      case 'trains':
        this.loadTrains();
        break;
      case 'logs':
        this.loadLogs();
        break;
    }
  }

  /**
   * 刷新仪表板
   */
  async refreshDashboard() {
    try {
      const response = await fetch('/api/admin/system/status');
      if (response.ok) {
        const data = await response.json();
        if (data.success) {
          this.updateDashboard(data.data);
        }
      }
    } catch (error) {
      console.error('刷新仪表板失败:', error);
    }
  }

  /**
   * 更新仪表板
   */
  updateDashboard(data) {
    // 更新系统健康状态
    this.updateHealthCards(data.health);
    
    // 更新通知统计
    this.updateNotificationStats(data.notifications);
    
    // 更新图表数据
    this.updateCharts(data);
  }

  /**
   * 更新健康卡片
   */
  updateHealthCards(health) {
    const statusElement = document.getElementById('systemStatus');
    if (statusElement) {
      statusElement.textContent = health.status === 'healthy' ? '正常' : '警告';
      statusElement.className = `badge ${health.status === 'healthy' ? 'bg-success' : 'bg-warning'}`;
    }

    // 更新各项指标
    Object.entries(health.checks).forEach(([key, check]) => {
      const element = document.getElementById(`${key}Status`);
      if (element) {
        element.textContent = check.value;
        element.className = `badge ${check.status === 'healthy' ? 'bg-success' : 'bg-warning'}`;
      }
    });
  }

  /**
   * 更新通知统计
   */
  updateNotificationStats(stats) {
    const elements = {
      totalUsers: document.getElementById('totalUsers'),
      onlineUsers: document.getElementById('onlineUsers'),
      totalNotifications: document.getElementById('totalNotifications'),
      unreadNotifications: document.getElementById('unreadNotifications')
    };

    Object.entries(elements).forEach(([key, element]) => {
      if (element && stats[key] !== undefined) {
        element.textContent = stats[key];
      }
    });
  }

  /**
   * 更新系统状态
   */
  updateSystemStatus(status) {
    this.systemStatus = status;
    
    // 更新性能图表
    if (this.charts.performance && status.cpu && status.memory) {
      const chart = this.charts.performance;
      const time = new Date(status.timestamp).toLocaleTimeString();
      
      chart.data.labels.push(time);
      chart.data.datasets[0].data.push(status.cpu.usage);
      chart.data.datasets[1].data.push(parseFloat(status.memory.system.usagePercent));
      
      // 保持最多20个数据点
      if (chart.data.labels.length > 20) {
        chart.data.labels.shift();
        chart.data.datasets.forEach(dataset => {
          dataset.data.shift();
        });
      }
      
      chart.update('none');
    }
  }

  /**
   * 更新连接状态
   */
  updateConnectionStatus(connected) {
    const statusElement = document.getElementById('connectionStatus');
    if (statusElement) {
      statusElement.textContent = connected ? '已连接' : '已断开';
      statusElement.className = `badge ${connected ? 'bg-success' : 'bg-danger'}`;
    }
  }

  /**
   * 显示通知
   */
  showNotification(notification) {
    const toast = document.createElement('div');
    toast.className = `toast align-items-center text-white bg-${this.getLevelClass(notification.level)} border-0`;
    toast.setAttribute('role', 'alert');
    toast.innerHTML = `
      <div class="d-flex">
        <div class="toast-body">
          <strong>${notification.title}</strong><br>
          ${notification.message}
        </div>
        <button type="button" class="btn-close btn-close-white me-2 m-auto" data-bs-dismiss="toast"></button>
      </div>
    `;

    const toastContainer = document.getElementById('toastContainer') || document.body;
    toastContainer.appendChild(toast);

    const bsToast = new bootstrap.Toast(toast);
    bsToast.show();

    // 自动移除
    toast.addEventListener('hidden.bs.toast', () => {
      toast.remove();
    });
  }

  /**
   * 获取级别对应的CSS类
   */
  getLevelClass(level) {
    const levelMap = {
      'success': 'success',
      'info': 'info',
      'warning': 'warning',
      'error': 'danger',
      'danger': 'danger'
    };
    return levelMap[level] || 'info';
  }

  /**
   * 处理添加列车
   */
  async handleAddTrain(form) {
    try {
      // 收集基本信息
      const formData = new FormData(form);
      const trainID = formData.get('trainID');
      const stationNum = parseInt(formData.get('stationNum'));
      const seatNum = parseInt(formData.get('seatNum'));
      const type = formData.get('type');
      const startTime = formData.get('startTime');
      const saleDate = formData.get('saleDate');

      // 验证基本信息
      if (!trainID || !stationNum || !seatNum || !type || !startTime || !saleDate) {
        this.showNotification({
          title: '错误',
          message: '请填写完整的列车信息',
          level: 'error'
        });
        return;
      }

      // 验证日期格式 (MM-DD|MM-DD)
      if (!/^(\d{2}-\d{2}\|)*\d{2}-\d{2}$/.test(saleDate)) {
        this.showNotification({
          title: '错误',
          message: '发售日期格式不正确，请使用MM-DD|MM-DD格式',
          level: 'error'
        });
        return;
      }

      // 收集车站信息
      const stations = [];
      const prices = [];
      const travelTimes = [];
      const stopoverTimes = [];

      for (let i = 0; i < stationNum; i++) {
        const stationName = formData.get(`station_${i}`);
        if (!stationName) {
          this.showNotification({
            title: '错误',
            message: `请填写第${i + 1}站的车站名称`,
            level: 'error'
          });
          return;
        }
        stations.push(stationName);

        if (i < stationNum - 1) {
          const price = parseFloat(formData.get(`price_${i}`));
          const travelTime = parseInt(formData.get(`travelTime_${i}`));
          
          if (isNaN(price) || price < 0) {
            this.showNotification({
              title: '错误',
              message: `请填写正确的第${i + 1}站到第${i + 2}站的票价`,
              level: 'error'
            });
            return;
          }
          
          if (isNaN(travelTime) || travelTime < 0) {
            this.showNotification({
              title: '错误',
              message: `请填写正确的第${i + 1}站到第${i + 2}站的行驶时间`,
              level: 'error'
            });
            return;
          }
          
          prices.push(price);
          travelTimes.push(travelTime);
        }

        if (i > 0 && i < stationNum - 1) {
          const stopover = parseInt(formData.get(`stopover_${i}`) || 5);
          stopoverTimes.push(stopover);
        }
      }

      // 构造请求数据
      const trainData = {
        trainId: trainID,
        stationNum,
        seatNum,
        stations: stations.join('|'),
        prices: prices.join('|'),
        startTime,
        travelTimes: travelTimes.join('|'),
        stopoverTimes: stopoverTimes.length > 0 ? stopoverTimes.join('|') : '_',
        saleDate,
        type
      };

      console.log('发送列车数据:', trainData);

      const response = await fetch('/api/admin/train/add', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(trainData)
      });

      const result = await response.json();
      
      if (result.success) {
        this.showNotification({
          title: '成功',
          message: '列车添加成功',
          level: 'success'
        });
        
        // 重置表单
        form.reset();
        document.getElementById('stationsContainer').innerHTML = '';
        
        // 关闭模态框
        const modal = bootstrap.Modal.getInstance(document.getElementById('addTrainModal'));
        if (modal) {
          modal.hide();
        }
        
        // 刷新列车列表
        this.loadTrains();
      } else {
        this.showNotification({
          title: '错误',
          message: result.message || '添加列车失败',
          level: 'error'
        });
      }
    } catch (error) {
      console.error('添加列车失败:', error);
      this.showNotification({
        title: '错误',
        message: '网络错误，请稍后重试',
        level: 'error'
      });
    }
  }

  /**
   * 处理广播消息
   */
  async handleBroadcastMessage(form) {
    const formData = new FormData(form);
    const message = formData.get('message');
    const level = formData.get('level') || 'info';

    if (!message.trim()) {
      this.showNotification({
        title: '错误',
        message: '消息内容不能为空',
        level: 'error'
      });
      return;
    }

    try {
      const response = await fetch('/api/admin/broadcast', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ message, level })
      });

      const result = await response.json();
      
      if (result.success) {
        this.showNotification({
          title: '成功',
          message: '消息已广播',
          level: 'success'
        });
        form.reset();
      } else {
        this.showNotification({
          title: '错误',
          message: result.message || '广播失败',
          level: 'error'
        });
      }
    } catch (error) {
      console.error('广播消息失败:', error);
      this.showNotification({
        title: '错误',
        message: '网络错误，请稍后重试',
        level: 'error'
      });
    }
  }

  /**
   * 加载用户列表
   */
  async loadUsers() {
    try {
      const response = await fetch('/api/admin/users');
      if (response.ok) {
        const data = await response.json();
        if (data.success) {
          this.renderUserTable(data.data);
        }
      }
    } catch (error) {
      console.error('加载用户列表失败:', error);
    }
  }

  /**
   * 加载订单列表
   */
  async loadOrders() {
    try {
      const response = await fetch('/api/admin/orders');
      if (response.ok) {
        const data = await response.json();
        if (data.success) {
          this.renderOrderTable(data.data);
        }
      }
    } catch (error) {
      console.error('加载订单列表失败:', error);
    }
  }

  /**
   * 加载列车列表
   */
  async loadTrains() {
    try {
      const response = await fetch('/api/admin/trains');
      if (response.ok) {
        const data = await response.json();
        if (data.success) {
          this.renderTrainTable(data.data);
        }
      }
    } catch (error) {
      console.error('加载列车列表失败:', error);
    }
  }

  /**
   * 按日期过滤订单
   */
  async filterOrdersByDate(mmddDate) {
    try {
      const response = await fetch(`/api/admin/orders?date=${mmddDate}`);
      if (response.ok) {
        const data = await response.json();
        if (data.success) {
          this.renderOrderTable(data.data);
        }
      }
    } catch (error) {
      console.error('按日期过滤订单失败:', error);
    }
  }

  /**
   * 渲染用户表格
   */
  renderUserTable(users) {
    const tbody = document.querySelector('#usersTable tbody');
    if (!tbody) return;

    tbody.innerHTML = users.map(user => `
      <tr>
        <td>
          <input type="checkbox" class="form-check-input item-checkbox" value="${user.username}">
        </td>
        <td>${user.username}</td>
        <td>${user.name || '-'}</td>
        <td>${user.mailAddr || '-'}</td>
        <td>
          <span class="badge ${user.privilege >= 10 ? 'bg-danger' : 'bg-primary'}">
            ${user.privilege >= 10 ? '管理员' : '普通用户'}
          </span>
        </td>
        <td>${new Date(user.createTime).toLocaleString()}</td>
        <td>
          <button class="btn btn-sm btn-outline-primary" onclick="adminPanel.viewUser('${user.username}')">
            <i class="fas fa-eye"></i>
          </button>
          <button class="btn btn-sm btn-outline-warning" onclick="adminPanel.editUser('${user.username}')">
            <i class="fas fa-edit"></i>
          </button>
          <button class="btn btn-sm btn-outline-danger" onclick="adminPanel.deleteUser('${user.username}')">
            <i class="fas fa-trash"></i>
          </button>
        </td>
      </tr>
    `).join('');
  }

  /**
   * 渲染订单表格
   */
  renderOrderTable(orders) {
    const tbody = document.querySelector('#ordersTable tbody');
    if (!tbody) return;

    tbody.innerHTML = orders.map(order => `
      <tr>
        <td>
          <input type="checkbox" class="form-check-input item-checkbox" value="${order.id}">
        </td>
        <td>${order.id}</td>
        <td>${order.username}</td>
        <td>${order.trainId}</td>
        <td>${order.fromStation} → ${order.toStation}</td>
        <td>${order.departureDate}</td>
        <td>${order.seatCount}</td>
        <td>¥${order.totalPrice}</td>
        <td>
          <span class="badge ${this.getOrderStatusClass(order.status)}">
            ${this.getOrderStatusText(order.status)}
          </span>
        </td>
        <td>${new Date(order.createTime).toLocaleString()}</td>
        <td>
          <button class="btn btn-sm btn-outline-primary" onclick="adminPanel.viewOrder('${order.id}')">
            <i class="fas fa-eye"></i>
          </button>
        </td>
      </tr>
    `).join('');
  }

  /**
   * 渲染列车表格
   */
  renderTrainTable(trains) {
    const tbody = document.querySelector('#trainsTable tbody');
    if (!tbody) return;

    tbody.innerHTML = trains.map(train => `
      <tr>
        <td>
          <input type="checkbox" class="form-check-input item-checkbox" value="${train.trainId}">
        </td>
        <td>${train.trainId}</td>
        <td>
          <span class="badge bg-info">${train.type}</span>
        </td>
        <td>${train.stationNum}</td>
        <td>${train.seatNum}</td>
        <td>${train.startTime}</td>
        <td>${train.saleDate}</td>
        <td>
          <span class="badge ${train.released ? 'bg-success' : 'bg-warning'}">
            ${train.released ? '已发布' : '未发布'}
          </span>
        </td>
        <td>
          <button class="btn btn-sm btn-outline-primary" onclick="adminPanel.viewTrain('${train.trainId}')">
            <i class="fas fa-eye"></i>
          </button>
          <button class="btn btn-sm btn-outline-warning" onclick="adminPanel.editTrain('${train.trainId}')">
            <i class="fas fa-edit"></i>
          </button>
          <button class="btn btn-sm btn-outline-danger" onclick="adminPanel.deleteTrain('${train.trainId}')">
            <i class="fas fa-trash"></i>
          </button>
        </td>
      </tr>
    `).join('');
  }

  /**
   * 获取订单状态对应的CSS类
   */
  getOrderStatusClass(status) {
    const statusMap = {
      'success': 'bg-success',
      'pending': 'bg-warning',
      'cancelled': 'bg-danger',
      'refunded': 'bg-info'
    };
    return statusMap[status] || 'bg-secondary';
  }

  /**
   * 获取订单状态文本
   */
  getOrderStatusText(status) {
    const statusMap = {
      'success': '已支付',
      'pending': '待支付',
      'cancelled': '已取消',
      'refunded': '已退票'
    };
    return statusMap[status] || '未知';
  }

  // 添加其他必需的方法存根
  searchUsers(query) {
    console.log('搜索用户:', query);
    // TODO: 实现用户搜索
  }

  searchOrders(query) {
    console.log('搜索订单:', query);
    // TODO: 实现订单搜索
  }

  searchTrains(query) {
    console.log('搜索列车:', query);
    // TODO: 实现列车搜索
  }

  updateBatchActions() {
    const selectedItems = document.querySelectorAll('.item-checkbox:checked');
    const batchActions = document.getElementById('batchActions');
    if (batchActions) {
      batchActions.style.display = selectedItems.length > 0 ? 'block' : 'none';
    }
  }

  viewUser(username) {
    console.log('查看用户:', username);
    // TODO: 实现查看用户详情
  }

  editUser(username) {
    console.log('编辑用户:', username);
    // TODO: 实现编辑用户
  }

  deleteUser(username) {
    console.log('删除用户:', username);
    // TODO: 实现删除用户
  }

  viewOrder(orderId) {
    console.log('查看订单:', orderId);
    // TODO: 实现查看订单详情
  }

  viewTrain(trainId) {
    console.log('查看列车:', trainId);
    // TODO: 实现查看列车详情
  }

  editTrain(trainId) {
    console.log('编辑列车:', trainId);
    // TODO: 实现编辑列车
  }

  deleteTrain(trainId) {
    console.log('删除列车:', trainId);
    // TODO: 实现删除列车
  }

  loadLogs() {
    console.log('加载日志');
    // TODO: 实现加载系统日志
  }
}

// 全局变量
let adminPanel;

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', () => {
  adminPanel = new AdminPanel();
  
  // 全局函数供HTML调用
  window.showTab = (tabName) => {
    adminPanel.showTab(tabName);
  };
  
  window.adminPanel = adminPanel;
});
