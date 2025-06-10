// 火车票管理系统前端脚本

(function() {
  'use strict';

  // 全局变量
  window.TicketSystem = {
    user: null,
    socket: null,
    notifications: {
      unreadCount: 0,
      list: []
    },
    config: {
      apiTimeout: 30000,
      retryCount: 3,
      debounceDelay: 300
    }
  };

  // 工具函数
  const Utils = {
    // 防抖函数
    debounce(func, wait) {
      let timeout;
      return function executedFunction(...args) {
        const later = () => {
          clearTimeout(timeout);
          func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
      };
    },

    // 节流函数
    throttle(func, limit) {
      let inThrottle;
      return function() {
        const args = arguments;
        const context = this;
        if (!inThrottle) {
          func.apply(context, args);
          inThrottle = true;
          setTimeout(() => inThrottle = false, limit);
        }
      };
    },

    // 格式化日期
    formatDate(date, format = 'YYYY-MM-DD HH:mm:ss') {
      const d = new Date(date);
      const year = d.getFullYear();
      const month = String(d.getMonth() + 1).padStart(2, '0');
      const day = String(d.getDate()).padStart(2, '0');
      const hour = String(d.getHours()).padStart(2, '0');
      const minute = String(d.getMinutes()).padStart(2, '0');
      const second = String(d.getSeconds()).padStart(2, '0');

      return format
        .replace('YYYY', year)
        .replace('MM', month)
        .replace('DD', day)
        .replace('HH', hour)
        .replace('mm', minute)
        .replace('ss', second);
    },

    // 验证日期格式
    validateDate(dateStr) {
      const regex = /^(\d{2})-(\d{2})$/;
      const match = dateStr.match(regex);
      if (!match) return false;

      const month = parseInt(match[1]);
      const day = parseInt(match[2]);
      
      return month >= 1 && month <= 12 && day >= 1 && day <= 31;
    },

    // 生成唯一ID
    generateId() {
      return Date.now().toString(36) + Math.random().toString(36).substr(2);
    },

    // 深拷贝对象
    deepClone(obj) {
      return JSON.parse(JSON.stringify(obj));
    },

    // 安全的JSON解析
    safeParse(str, defaultValue = null) {
      try {
        return JSON.parse(str);
      } catch (e) {
        return defaultValue;
      }
    }
  };

  // HTTP请求封装
  const HTTP = {
    async request(url, options = {}) {
      const defaultOptions = {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json',
        },
        timeout: TicketSystem.config.apiTimeout
      };

      const config = { ...defaultOptions, ...options };
      
      // 添加超时控制
      const controller = new AbortController();
      const timeoutId = setTimeout(() => controller.abort(), config.timeout);
      config.signal = controller.signal;

      try {
        showLoading();
        const response = await fetch(url, config);
        clearTimeout(timeoutId);
        
        if (!response.ok) {
          throw new Error(`HTTP Error: ${response.status} ${response.statusText}`);
        }
        
        const data = await response.json();
        return data;
      } catch (error) {
        clearTimeout(timeoutId);
        if (error.name === 'AbortError') {
          throw new Error('请求超时，请重试');
        }
        throw error;
      } finally {
        hideLoading();
      }
    },

    get(url, params = {}) {
      const urlParams = new URLSearchParams(params);
      const fullUrl = urlParams.toString() ? `${url}?${urlParams}` : url;
      return this.request(fullUrl);
    },

    post(url, data = {}) {
      return this.request(url, {
        method: 'POST',
        body: JSON.stringify(data)
      });
    },

    put(url, data = {}) {
      return this.request(url, {
        method: 'PUT',
        body: JSON.stringify(data)
      });
    },

    delete(url) {
      return this.request(url, {
        method: 'DELETE'
      });
    }
  };

  // 消息提示系统
  const Message = {
    show(content, type = 'info', duration = 5000) {
      // 移除现有的提示
      this.hide();

      const messageContainer = document.getElementById('messageContainer');
      if (!messageContainer) return;

      const alertClass = {
        'info': 'alert-info',
        'success': 'alert-success',
        'warning': 'alert-warning',
        'danger': 'alert-danger',
        'error': 'alert-danger'
      }[type] || 'alert-info';

      const iconClass = {
        'info': 'fa-info-circle',
        'success': 'fa-check-circle',
        'warning': 'fa-exclamation-triangle',
        'danger': 'fa-exclamation-circle',
        'error': 'fa-exclamation-circle'
      }[type] || 'fa-info-circle';

      const alertHtml = `
        <div class="alert ${alertClass} alert-dismissible fade show slide-in-left" role="alert" id="dynamicAlert">
          <i class="fas ${iconClass}"></i>
          <strong>${this.getTypeLabel(type)}：</strong> ${content}
          <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
        </div>
      `;

      messageContainer.insertAdjacentHTML('afterbegin', alertHtml);

      // 自动隐藏
      if (duration > 0) {
        setTimeout(() => this.hide(), duration);
      }
    },

    success(content, duration) {
      this.show(content, 'success', duration);
    },

    error(content, duration) {
      this.show(content, 'error', duration);
    },

    warning(content, duration) {
      this.show(content, 'warning', duration);
    },

    info(content, duration) {
      this.show(content, 'info', duration);
    },

    hide() {
      const alert = document.getElementById('dynamicAlert');
      if (alert) {
        alert.remove();
      }
    },

    getTypeLabel(type) {
      const labels = {
        'info': '信息',
        'success': '成功',
        'warning': '警告',
        'danger': '错误',
        'error': '错误'
      };
      return labels[type] || '提示';
    }
  };

  // 加载状态管理
  function showLoading(text = '正在处理...') {
    let overlay = document.getElementById('loadingOverlay');
    if (!overlay) {
      overlay = document.createElement('div');
      overlay.id = 'loadingOverlay';
      overlay.className = 'loading-overlay';
      overlay.innerHTML = `
        <div class="spinner-border text-primary" role="status">
          <span class="visually-hidden">加载中...</span>
        </div>
        <div class="mt-2" id="loadingText">${text}</div>
      `;
      document.body.appendChild(overlay);
    }
    
    document.getElementById('loadingText').textContent = text;
    overlay.style.display = 'flex';
  }

  function hideLoading() {
    const overlay = document.getElementById('loadingOverlay');
    if (overlay) {
      overlay.style.display = 'none';
    }
  }

  // 表单验证
  const Validator = {
    rules: {
      required: (value) => value.trim() !== '',
      email: (value) => /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(value),
      username: (value) => /^[a-zA-Z][a-zA-Z0-9_]*$/.test(value) && value.length >= 1 && value.length <= 20,
      date: (value) => Utils.validateDate(value),
      number: (value) => !isNaN(value) && isFinite(value),
      integer: (value) => Number.isInteger(Number(value)),
      min: (value, min) => Number(value) >= min,
      max: (value, max) => Number(value) <= max,
      minLength: (value, length) => value.length >= length,
      maxLength: (value, length) => value.length <= length
    },

    validate(form) {
      const errors = [];
      const elements = form.querySelectorAll('[data-validate]');

      elements.forEach(element => {
        const rules = element.dataset.validate.split('|');
        const value = element.value.trim();
        const label = element.dataset.label || element.name || '字段';

        rules.forEach(rule => {
          const [ruleName, ruleValue] = rule.split(':');
          
          if (this.rules[ruleName]) {
            const isValid = ruleValue ? 
              this.rules[ruleName](value, ruleValue) : 
              this.rules[ruleName](value);
            
            if (!isValid) {
              errors.push(this.getErrorMessage(label, ruleName, ruleValue));
              element.classList.add('is-invalid');
            } else {
              element.classList.remove('is-invalid');
              element.classList.add('is-valid');
            }
          }
        });
      });

      return {
        isValid: errors.length === 0,
        errors: errors
      };
    },

    getErrorMessage(label, rule, value) {
      const messages = {
        required: `${label}不能为空`,
        email: `${label}格式不正确`,
        username: `${label}必须以字母开头，只能包含字母、数字和下划线`,
        date: `${label}格式不正确，请使用MM-DD格式`,
        number: `${label}必须是数字`,
        integer: `${label}必须是整数`,
        min: `${label}不能小于${value}`,
        max: `${label}不能大于${value}`,
        minLength: `${label}长度不能少于${value}个字符`,
        maxLength: `${label}长度不能超过${value}个字符`
      };
      return messages[rule] || `${label}验证失败`;
    }
  };

  // 通知系统
  const NotificationManager = {
    init() {
      this.createNotificationContainer();
      this.bindEvents();
      this.loadNotifications();
    },

    createNotificationContainer() {
      if (!document.getElementById('notificationContainer')) {
        const container = document.createElement('div');
        container.id = 'notificationContainer';
        container.className = 'notification-container position-fixed top-0 end-0 p-3';
        container.style.zIndex = '9999';
        document.body.appendChild(container);
      }
    },

    bindEvents() {
      // 通知铃铛点击事件
      const notificationBell = document.getElementById('notificationBell');
      if (notificationBell) {
        notificationBell.addEventListener('click', () => {
          this.toggleNotificationDropdown();
        });
      }

      // 标记所有为已读
      const markAllReadBtn = document.getElementById('markAllRead');
      if (markAllReadBtn) {
        markAllReadBtn.addEventListener('click', () => {
          this.markAllAsRead();
        });
      }
    },

    async loadNotifications() {
      try {
        const response = await fetch('/api/notifications');
        if (response.ok) {
          const data = await response.json();
          if (data.success) {
            TicketSystem.notifications.list = data.data.notifications;
            TicketSystem.notifications.unreadCount = data.data.unreadCount;
            this.updateUI();
          }
        }
      } catch (error) {
        console.error('加载通知失败:', error);
      }
    },

    updateUI() {
      this.updateBadge();
      this.updateDropdown();
    },

    updateBadge() {
      const badge = document.getElementById('notificationBadge');
      if (badge) {
        if (TicketSystem.notifications.unreadCount > 0) {
          badge.textContent = TicketSystem.notifications.unreadCount > 99 ? '99+' : TicketSystem.notifications.unreadCount;
          badge.style.display = 'inline-block';
        } else {
          badge.style.display = 'none';
        }
      }
    },

    updateDropdown() {
      const dropdown = document.getElementById('notificationDropdown');
      if (!dropdown) return;

      const notifications = TicketSystem.notifications.list;
      const container = dropdown.querySelector('.notification-list');
      
      if (notifications.length === 0) {
        container.innerHTML = '<div class="text-center text-muted p-3">暂无通知</div>';
        return;
      }

      container.innerHTML = notifications.slice(0, 10).map(notification => `
        <div class="notification-item ${notification.read ? '' : 'unread'}" data-id="${notification.id}">
          <div class="d-flex align-items-start">
            <div class="notification-icon me-2">
              <i class="fas ${this.getNotificationIcon(notification.type)} text-${this.getNotificationColor(notification.level)}"></i>
            </div>
            <div class="flex-grow-1">
              <div class="notification-title fw-bold">${notification.title}</div>
              <div class="notification-message text-muted small">${notification.message}</div>
              <div class="notification-time text-muted small">${Utils.formatDate(notification.timestamp, 'MM-DD HH:mm')}</div>
            </div>
            ${!notification.read ? '<div class="unread-indicator bg-primary rounded-circle"></div>' : ''}
          </div>
        </div>
      `).join('');

      // 绑定点击事件
      container.querySelectorAll('.notification-item').forEach(item => {
        item.addEventListener('click', () => {
          this.markAsRead(item.dataset.id);
        });
      });
    },

    getNotificationIcon(type) {
      const icons = {
        'system': 'fa-cog',
        'order': 'fa-shopping-cart',
        'ticket': 'fa-ticket-alt',
        'admin': 'fa-user-shield',
        'announcement': 'fa-bullhorn',
        'maintenance': 'fa-tools'
      };
      return icons[type] || 'fa-bell';
    },

    getNotificationColor(level) {
      const colors = {
        'success': 'success',
        'info': 'info',
        'warning': 'warning',
        'error': 'danger',
        'danger': 'danger'
      };
      return colors[level] || 'info';
    },

    async markAsRead(notificationId) {
      try {
        const response = await fetch(`/api/notifications/${notificationId}/read`, {
          method: 'POST'
        });
        
        if (response.ok) {
          const notification = TicketSystem.notifications.list.find(n => n.id === notificationId);
          if (notification && !notification.read) {
            notification.read = true;
            TicketSystem.notifications.unreadCount = Math.max(0, TicketSystem.notifications.unreadCount - 1);
            this.updateUI();
          }
        }
      } catch (error) {
        console.error('标记通知已读失败:', error);
      }
    },

    async markAllAsRead() {
      const unreadNotifications = TicketSystem.notifications.list.filter(n => !n.read);
      
      for (const notification of unreadNotifications) {
        await this.markAsRead(notification.id);
      }
    },

    showToast(notification) {
      const toast = document.createElement('div');
      toast.className = `toast align-items-center text-white bg-${this.getNotificationColor(notification.level)} border-0`;
      toast.setAttribute('role', 'alert');
      toast.setAttribute('aria-live', 'assertive');
      toast.setAttribute('aria-atomic', 'true');
      
      toast.innerHTML = `
        <div class="d-flex">
          <div class="toast-body">
            <div class="d-flex align-items-center">
              <i class="fas ${this.getNotificationIcon(notification.type)} me-2"></i>
              <div>
                <div class="fw-bold">${notification.title}</div>
                <div class="small">${notification.message}</div>
              </div>
            </div>
          </div>
          <button type="button" class="btn-close btn-close-white me-2 m-auto" data-bs-dismiss="toast"></button>
        </div>
      `;

      const container = document.getElementById('notificationContainer');
      container.appendChild(toast);

      const bsToast = new bootstrap.Toast(toast, {
        autohide: true,
        delay: 5000
      });
      bsToast.show();

      toast.addEventListener('hidden.bs.toast', () => {
        toast.remove();
      });
    },

    addNotification(notification) {
      TicketSystem.notifications.list.unshift(notification);
      if (!notification.read) {
        TicketSystem.notifications.unreadCount++;
      }
      this.updateUI();
      this.showToast(notification);
    }
  };

  // WebSocket连接管理
  const SocketManager = {
    socket: null,
    reconnectAttempts: 0,
    maxReconnectAttempts: 5,
    reconnectDelay: 1000,

    init() {
      this.connect();
    },

    connect() {
      if (typeof io === 'undefined') {
        console.warn('Socket.IO未加载');
        return;
      }

      this.socket = io();
      TicketSystem.socket = this.socket;

      this.socket.on('connect', () => {
        console.log('WebSocket连接成功');
        this.reconnectAttempts = 0;
        this.updateConnectionStatus(true);
        
        // 用户认证
        if (window.currentUser) {
          this.socket.emit('user_auth', window.currentUser);
        }
      });

      this.socket.on('disconnect', () => {
        console.log('WebSocket连接断开');
        this.updateConnectionStatus(false);
        this.scheduleReconnect();
      });

      this.socket.on('connect_error', (error) => {
        console.error('WebSocket连接错误:', error);
        this.updateConnectionStatus(false);
        this.scheduleReconnect();
      });

      // 认证成功
      this.socket.on('auth_success', (data) => {
        console.log('用户认证成功:', data);
      });

      // 新通知
      this.socket.on('new_notification', (notification) => {
        NotificationManager.addNotification(notification);
      });

      // 未读通知
      this.socket.on('unread_notifications', (notifications) => {
        TicketSystem.notifications.list = notifications;
        TicketSystem.notifications.unreadCount = notifications.filter(n => !n.read).length;
        NotificationManager.updateUI();
      });

      // 基础状态更新
      this.socket.on('basic_status', (status) => {
        this.updateBasicStatus(status);
      });

      // 在线用户数更新
      this.socket.on('online_users', (count) => {
        this.updateOnlineUsers(count);
      });
    },

    scheduleReconnect() {
      if (this.reconnectAttempts < this.maxReconnectAttempts) {
        this.reconnectAttempts++;
        const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1);
        
        console.log(`${delay}ms后尝试重连... (${this.reconnectAttempts}/${this.maxReconnectAttempts})`);
        
        setTimeout(() => {
          this.connect();
        }, delay);
      } else {
        console.error('WebSocket重连失败，已达到最大重试次数');
      }
    },

    updateConnectionStatus(connected) {
      const indicator = document.getElementById('connectionIndicator');
      if (indicator) {
        indicator.className = `connection-indicator ${connected ? 'connected' : 'disconnected'}`;
        indicator.title = connected ? '已连接' : '连接断开';
      }
    },

    updateBasicStatus(status) {
      const elements = {
        systemUptime: document.getElementById('systemUptime'),
        systemMemory: document.getElementById('systemMemory'),
        systemCPU: document.getElementById('systemCPU')
      };

      if (elements.systemUptime) {
        elements.systemUptime.textContent = status.uptime;
      }
      if (elements.systemMemory) {
        elements.systemMemory.textContent = status.memory;
      }
      if (elements.systemCPU) {
        elements.systemCPU.textContent = `${status.cpu}%`;
      }
    },

    updateOnlineUsers(count) {
      const element = document.getElementById('onlineUsersCount');
      if (element) {
        element.textContent = count;
      }
    },

    disconnect() {
      if (this.socket) {
        this.socket.disconnect();
        this.socket = null;
        TicketSystem.socket = null;
      }
    }
  };

  // 页面性能监控
  const PerformanceMonitor = {
    init() {
      this.monitorPageLoad();
      this.monitorUserInteraction();
    },

    monitorPageLoad() {
      if ('performance' in window) {
        window.addEventListener('load', () => {
          setTimeout(() => {
            const timing = performance.timing;
            const loadTime = timing.loadEventEnd - timing.navigationStart;
            
            if (loadTime > 0) {
              console.log(`页面加载时间: ${loadTime}ms`);
              
              // 如果加载时间过长，显示提示
              if (loadTime > 5000) {
                Message.warning('页面加载较慢，建议检查网络连接');
              }
            }
          }, 0);
        });
      }
    },

    monitorUserInteraction() {
      // 监控长时间无操作
      let lastActivity = Date.now();
      const IDLE_TIMEOUT = 30 * 60 * 1000; // 30分钟

      const resetActivity = () => {
        lastActivity = Date.now();
      };

      ['mousedown', 'mousemove', 'keypress', 'scroll', 'touchstart'].forEach(event => {
        document.addEventListener(event, resetActivity, true);
      });

      // 定期检查空闲状态
      setInterval(() => {
        const idleTime = Date.now() - lastActivity;
        if (idleTime > IDLE_TIMEOUT) {
          this.handleIdleTimeout();
        }
      }, 60000); // 每分钟检查一次
    },

    handleIdleTimeout() {
      if (window.currentUser) {
        Message.info('您已长时间未操作，为了安全起见，请重新验证身份', {
          duration: 0,
          showClose: true
        });
      }
    }
  };

  // 更新初始化函数
  function initializePage() {
    console.log('初始化火车票系统前端...');
    
    // 初始化各个组件
    FormHandler.init();
    LoadingManager.init();
    ResponsiveTable.init();
    NotificationManager.init();
    SocketManager.init();
    PerformanceMonitor.init();
    
    // 绑定全局事件
    bindGlobalEvents();
    
    console.log('火车票系统前端初始化完成');
  }

  // 页面卸载时清理
  window.addEventListener('beforeunload', () => {
    SocketManager.disconnect();
  });

  // 导出新的全局函数
  window.NotificationManager = NotificationManager;
  window.SocketManager = SocketManager;

  // DOM加载完成后初始化
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initializePage);
  } else {
    initializePage();
  }

})();
